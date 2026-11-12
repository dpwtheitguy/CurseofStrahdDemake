"""Run mGBA via libretro and verify the rendered 240x160 boot screen."""

import ctypes as c
import hashlib
import struct
import sys
import zlib
from pathlib import Path

WIDTH, HEIGHT = 240, 160
EXPECTED_SHA256 = "8e6af71e5a68a89b28d7dbaa41d871280f1f5418645e365578bfa7adcfed3f0b"

Environment = c.CFUNCTYPE(c.c_bool, c.c_uint, c.c_void_p)
Video = c.CFUNCTYPE(None, c.c_void_p, c.c_uint, c.c_uint, c.c_size_t)
Audio = c.CFUNCTYPE(None, c.c_int16, c.c_int16)
AudioBatch = c.CFUNCTYPE(c.c_size_t, c.c_void_p, c.c_size_t)
InputPoll = c.CFUNCTYPE(None)
InputState = c.CFUNCTYPE(c.c_int16, c.c_uint, c.c_uint, c.c_uint, c.c_uint)


class GameInfo(c.Structure):
    _fields_ = [
        ("path", c.c_char_p),
        ("data", c.c_void_p),
        ("size", c.c_size_t),
        ("meta", c.c_char_p),
    ]


def render(core_path, rom_path, inputs=None, *, frames=None):
    core = c.CDLL(str(core_path.resolve()))
    pixel_format = 0  # Default libretro 0RGB1555.
    frame = None
    dimensions = None

    @Environment
    def environment(command, data):
        nonlocal pixel_format
        if command == 10:  # RETRO_ENVIRONMENT_SET_PIXEL_FORMAT
            requested = c.cast(data, c.POINTER(c.c_int))[0]
            if requested in (0, 1, 2):
                pixel_format = requested
                return True
        return False

    @Video
    def video(data, width, height, pitch):
        nonlocal frame, dimensions
        if not data:
            return
        dimensions = width, height
        if dimensions != (WIDTH, HEIGHT):
            frame = None
            return

        stride = 4 if pixel_format == 1 else 2
        frame = b"".join(
            c.string_at(data + y * pitch, width * stride) for y in range(height)
        )

    @InputState
    def input_state(port, device, index, ident):
        return int(port == 0 and device == 1 and index == 0 and ident in buttons)

    buttons = ()
    callbacks = {
        "environment": environment,
        "video_refresh": video,
        "audio_sample": Audio(lambda left, right: None),
        "audio_sample_batch": AudioBatch(lambda data, frames: frames),
        "input_poll": InputPoll(lambda: None),
        "input_state": input_state,
    }
    for name, callback in callbacks.items():
        setter = getattr(core, f"retro_set_{name}")
        setter.argtypes = [type(callback)]
        setter.restype = None
        setter(callback)

    core.retro_load_game.argtypes = [c.POINTER(GameInfo)]
    core.retro_load_game.restype = c.c_bool
    for name in ("retro_init", "retro_run", "retro_unload_game", "retro_deinit"):
        function = getattr(core, name)
        function.argtypes = []
        function.restype = None
    rom = c.create_string_buffer(rom_path.read_bytes())
    game = GameInfo(bytes(rom_path.resolve()), c.addressof(rom), len(rom) - 1, None)
    core.retro_init()
    try:
        if not core.retro_load_game(c.byref(game)):
            raise SystemExit("mGBA could not load the ROM")
        try:
            if frames is None:
                frames = max(120, max(inputs or {0: ()}) + 10)
            for frame_number in range(frames):
                buttons = inputs.get(frame_number, ()) if inputs else ()
                core.retro_run()
        finally:
            core.retro_unload_game()
    finally:
        core.retro_deinit()

    if frame is None:
        raise SystemExit(f"No {WIDTH}x{HEIGHT} frame from mGBA (got {dimensions})")
    return rgb_bytes(frame, pixel_format)


def rgb_bytes(frame, pixel_format):
    # Normalize the core's output to the GBA's five bits per channel.
    if pixel_format == 1:
        pixels = ((v >> 19 & 31, v >> 11 & 31, v >> 3 & 31)
                  for (v,) in struct.iter_unpack("=I", frame))
    elif pixel_format == 2:
        pixels = ((v >> 11 & 31, v >> 6 & 31, v & 31)
                  for (v,) in struct.iter_unpack("=H", frame))
    else:
        pixels = ((v >> 10 & 31, v >> 5 & 31, v & 31)
                  for (v,) in struct.iter_unpack("=H", frame))
    return bytes((v << 3) | (v >> 2) for pixel in pixels for v in pixel)


def write_png(path, rgb):
    def chunk(kind, payload):
        return (struct.pack(">I", len(payload)) + kind + payload
                + struct.pack(">I", zlib.crc32(kind + payload)))

    row_bytes = WIDTH * 3
    rows = b"".join(b"\0" + rgb[i:i + row_bytes] for i in range(0, len(rgb), row_bytes))
    header = struct.pack(">IIBBBBB", WIDTH, HEIGHT, 8, 2, 0, 0, 0)
    path.write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header)
                     + chunk(b"IDAT", zlib.compress(rows)) + chunk(b"IEND", b""))


if __name__ == "__main__":
    core_path, rom_path, screenshot = map(Path, sys.argv[1:])
    screenshot.unlink(missing_ok=True)
    if not core_path.is_file():
        raise SystemExit(f"mGBA core missing at {core_path}. Run bootstrap or set MGBA_CORE.")
    rgb = render(core_path, rom_path)
    write_png(screenshot, rgb)
    digest = hashlib.sha256(rgb).hexdigest()
    if digest != EXPECTED_SHA256:
        raise SystemExit(f"Unexpected frame SHA-256 {digest}. Inspect {screenshot}.")
    print(f"mGBA boot screen OK ({WIDTH}x{HEIGHT}). Screenshot at {screenshot}")
