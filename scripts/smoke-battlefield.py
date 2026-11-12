"""Exercise the scrolling battlefields through the emulator input callback."""

import re
import sys
from pathlib import Path

from smoke import HEIGHT, WIDTH, render, write_png

B, SELECT, START, UP, DOWN, LEFT, RIGHT, A = 0, 2, 3, 4, 5, 6, 7, 8
CURSOR_GOLD = bytes((255, 247, 148))


def cursor_corners(rgb):
    return {
        (i % WIDTH, i // WIDTH)
        for i in range(WIDTH * HEIGHT)
        if rgb[3 * i:3 * i + 3] == CURSOR_GOLD
    }


def check_cursor(frame, x, y):
    left, top = x * 16, y * 16
    gold = cursor_corners(frame)
    assert len(gold) == 44, "Expected one cursor with four gold brackets"
    assert all(left <= px < left + 16 and top <= py < top + 16 for px, py in gold)
    assert {(left, top), (left + 15, top + 15)} <= gold


def check_terrain(before, after, old, new):
    for y in range(HEIGHT):
        for x in range(WIDTH):
            if (x // 16, y // 16) in (old, new):
                continue
            i = 3 * (y * WIDTH + x)
            assert before[i:i + 3] == after[i:i + 3], "Cursor movement changed terrain"


# Every terrain has a distinct set of colors. Check its center pixel, which
# stays visible even when the cursor surrounds the square.
TERRAIN_COLORS = {
    ".": ((8, 13, 8), (6, 10, 7), (11, 16, 9)),
    "=": ((17, 13, 8), (14, 10, 7), (20, 16, 11)),
    "#": ((7, 8, 10), (16, 17, 18), (20, 21, 21), (11, 12, 14), (24, 24, 22)),
    "~": ((4, 10, 15), (5, 12, 18), (8, 16, 21)),
    "T": ((3, 8, 6), (5, 12, 7), (10, 17, 8)),
    ",": ((8, 11, 9), (12, 14, 8), (7, 13, 12)),
    ":": ((10, 10, 12), (15, 15, 17), (19, 18, 20)),
    "+": ((11, 7, 5), (21, 14, 8), (25, 18, 11)),
}


def check_view(frame, rows, camera_x, camera_y):
    for y in range(HEIGHT // 16):
        for x in range(WIDTH // 16):
            world_x, world_y = camera_x + x, camera_y + y
            symbol = rows[world_y][world_x]
            offset = 3 * ((16 * y + 8) * WIDTH + 16 * x + 8)
            color = tuple(channel >> 3 for channel in frame[offset:offset + 3])
            assert color in TERRAIN_COLORS[symbol], f"Incorrect terrain at {(world_x, world_y)}"


def check_fighter(frame, x, y, counter, swim, in_water=False):
    def color_at(sprite_x, sprite_y):
        offset = 3 * ((16 * y + sprite_y) * WIDTH + 16 * x + sprite_x)
        return tuple(v >> 3 for v in frame[offset:offset + 3])

    if in_water:
        assert color_at(1, 10) == (26, 30, 30), "Missing swimming wake"
        assert color_at(5, 14) in TERRAIN_COLORS["~"], "Standing boots visible underwater"
    else:
        assert color_at(8, 6) == (19, 5, 6), "Missing standing fighter"
        assert color_at(5, 14) == (7, 8, 11), "Missing standing boots"
    glyphs = {
        "0": (14, 17, 19, 21, 25, 17, 14),
        "1": (4, 12, 4, 4, 4, 4, 14),
        "2": (14, 17, 1, 2, 4, 8, 31),
        "3": (30, 1, 1, 14, 1, 1, 30),
        "5": (31, 16, 16, 30, 1, 1, 30),
        "N": (17, 25, 21, 19, 17, 17, 17),
        "F": (31, 16, 16, 30, 16, 16, 16),
    }
    for column, letter in ((6, counter[0]), (7, counter[1]), (25, "N" if swim else "F")):
        for y, row in enumerate(glyphs[letter]):
            for x in range(5):
                offset = 3 * (y * WIDTH + 8 * column + 1 + x)
                color = tuple(v >> 3 for v in frame[offset:offset + 3])
                assert color == ((31, 29, 22) if row & (16 >> x) else (4, 5, 8))


def check_animation(core, rom, enter):
    swim_inputs = enter | {50: (RIGHT,)}
    glide = render(core, rom, swim_inputs, frames=120)
    stroke = render(core, rom, swim_inputs, frames=128)
    repeated = render(core, rom, swim_inputs, frames=136)
    for frame in (glide, stroke):
        check_fighter(frame, 10, 4, "10", False, True)
    assert glide != stroke and glide == repeated, "Swim poses should repeat every 16 frames"
    check_terrain(glide, stroke, (10, 4), (10, 4))

    walk_inputs = enter | {50: (DOWN,)}
    stepping = render(core, rom, walk_inputs, frames=54)
    standing = render(core, rom, walk_inputs, frames=70)
    check_fighter(standing, 9, 5, "05", False)
    boot = 3 * ((5 * 16 + 15) * WIDTH + 9 * 16 + 10)
    assert stepping[boot:boot + 3] != standing[boot:boot + 3], "A step should lift one boot"
    check_terrain(stepping, standing, (9, 4), (9, 5))

    def tunic_top(frame):
        return next(y for y in range(64, 96)
                    if tuple(v >> 3 for v in frame[3 * (y * WIDTH + 152):
                                                   3 * (y * WIDTH + 152) + 3]) == (19, 5, 6))

    assert 70 < tunic_top(stepping) < tunic_top(standing), "Expected an in-between sprite position"
    idle = render(core, rom, enter, frames=120)
    breath = render(core, rom, enter, frames=152)
    assert idle != breath and idle == render(core, rom, enter, frames=184)
    check_terrain(idle, breath, (9, 4), (9, 4))


def check_camera_animation(core, rom):
    inputs = {36: (SELECT,)} | {frame: (RIGHT,) for frame in (40, 46, 52, 58, 64)}
    before = render(core, rom, inputs, frames=64)
    for frame_number, scroll in ((66, 4), (67, 8), (68, 12), (69, 16)):
        frame = render(core, rom, inputs, frames=frame_number)
        # The ruin is well away from the moving sprite and the fixed HUD.
        for y in range(17, 46):
            start = 3 * (y * WIDTH + 20)
            end = 3 * (y * WIDTH + 100)
            assert frame[start:end] == before[start + 3 * scroll:end + 3 * scroll]


def read_status(frame, tile_row=1):
    art = (Path(__file__).resolve().parents[1] / "src/battlefield-fighter.h").read_text()
    shapes = re.findall(r"\{([\d, ]+)\},\s*/\*", art)
    letters = re.search(r'hud_letters\[\] = "([^"]+)"', art)[1]
    glyphs = {tuple(map(int, shape.split(","))): letter
              for shape, letter in zip(shapes, letters, strict=True)}
    status = []
    for column in range(30):
        pixels = []
        for row in range(7):
            bits = 0
            for x in range(5):
                offset = 3 * ((row + tile_row * 8) * WIDTH + column * 8 + 1 + x)
                bits = (bits << 1) | (frame[offset] >> 3 == 31)
            pixels.append(bits)
        status.append(glyphs[tuple(pixels)])
    return "".join(status).strip()


def check_fence(core, rom, screenshot):
    def intact(frame):
        offset = 3 * (54 * WIDTH + 152)  # Middle of the upper wooden rail.
        return tuple(v >> 3 for v in frame[offset:offset + 3]) == (21, 12, 6)

    blocked = render(core, rom, {50: (UP,)})
    assert intact(blocked) and read_status(blocked) == "X SELECT OBJECT"
    check_fighter(blocked, 9, 4, "00", False)

    def pixel(frame, x, y):
        offset = 3 * (y * WIDTH + x)
        return tuple(v >> 3 for v in frame[offset:offset + 3])

    open_menu = {46: (A,)}
    selected = render(core, rom, open_menu)
    assert intact(selected)
    assert pixel(selected, 144, 48) == (31, 30, 18), "Expected gold fence brackets"
    assert pixel(selected, 73, 118) == (31, 25, 11), "Expected gold Smash frame"
    assert pixel(selected, 137, 118) == (24, 15, 6), "Expected bronze Back frame"
    assert pixel(selected, 105, 94) == (24, 15, 6), "Expected a third card for the target"
    assert pixel(selected, 96, 98) == pixel(blocked, 96, 98), "Expected terrain between cards"
    write_png(screenshot.with_name("fence-selector.png"), selected)

    cancel_selected = render(core, rom, open_menu | {48: (RIGHT,)})
    assert pixel(cancel_selected, 73, 118) == (24, 15, 6)
    assert pixel(cancel_selected, 137, 118) == (31, 25, 11)
    check_fighter(cancel_selected, 9, 4, "00", False)
    assert render(core, rom, open_menu | {48: (RIGHT,), 50: (A,)}) == blocked
    assert render(core, rom, open_menu | {50: (B,)}) == blocked
    assert render(core, rom, open_menu | {48: (RIGHT,), 50: (LEFT,)}) == selected

    for frame_number, success in ((51, True), (53, False)):
        inputs = open_menu | {frame_number: (A,)}
        closeup = render(core, rom, inputs, frames=frame_number + 42)
        attempt = render(core, rom, inputs | {frame_number + 44: (A,)})
        assert read_status(closeup, 18) == read_status(attempt)
        assert read_status(closeup, 19) == "X NEXT"
        if success:
            windup = render(core, rom, inputs, frames=frame_number + 12)
            impact = render(core, rom, inputs, frames=frame_number + 28)
            assert read_status(windup, 18) == "SMASH"
            assert len({windup, impact, closeup}) == 3
            assert render(core, rom, inputs, frames=frame_number + 170) == closeup
            assert render(core, rom, inputs | {frame_number + 12: (UP,)},
                          frames=frame_number + 42) == closeup
            write_png(screenshot.with_name("smash-closeup.png"), closeup)
        result = re.fullmatch(r"D20 (\d{2}) \+4 = (\d{2}) DC12 (BROKE|FAIL)", read_status(attempt))
        assert result, "Expected the Athletics die, total, DC and result"
        die, total = map(int, result.groups()[:2])
        assert 1 <= die <= 20 and total == die + 4
        assert (total >= 12) == success == (result[3] == "BROKE")
        assert intact(attempt) != success
        check_fighter(attempt, 9, 4, "00", False)

    failed = render(core, rom, open_menu | {53: (A,), 97: (A,), 101: (A,), 105: (UP,)})
    assert intact(failed) and read_status(failed).endswith("FAIL"), "No second attempt in one turn"
    check_fighter(failed, 9, 4, "00", False)

    opened = render(core, rom, open_menu | {51: (A,), 95: (A,), 101: (UP,)})
    check_fighter(opened, 9, 3, "05", False)
    assert read_status(opened).endswith("BROKE")
    write_png(screenshot.with_name("fence-broken.png"), opened)

    retry = render(core, rom, open_menu | {53: (A,), 97: (A,), 102: (B,), 106: (A,), 111: (A,), 155: (A,)})
    assert not intact(retry) and read_status(retry).endswith("BROKE")

    out_of_reach = render(core, rom, {40: (LEFT,), 44: (LEFT,), 50: (A,)})
    assert intact(out_of_reach) and read_status(out_of_reach) == "MOVE NEXT TO OBJECT"
    check_fighter(out_of_reach, 7, 4, "10", False)

    reset = render(core, rom, open_menu | {51: (A,), 95: (A,), 100: (START,), 110: (START,)})
    assert intact(reset) and read_status(reset) == "X SELECT OBJECT"
    print("Fence OK. Target brackets, command selection, cancel and Athletics results verified.")


def check_other_breakables(core, rom):
    # A total of 12 clears the crate's DC 10 but fails the barrel's DC 14.
    crate = {40: (DOWN,), 46: (A,), 59: (A,), 103: (A,)}
    barrel = {40: (DOWN,), 42: (RIGHT,), 46: (A,), 59: (A,), 103: (A,)}
    crate_result = render(core, rom, crate | {108: (DOWN,)})
    barrel_result = render(core, rom, barrel | {108: (RIGHT,)})
    assert read_status(crate_result) == "D20 08 +4 = 12 DC10 BROKE"
    assert read_status(barrel_result) == "D20 08 +4 = 12 DC14 FAIL"
    check_fighter(crate_result, 9, 6, "10", False)
    check_fighter(barrel_result, 10, 5, "10", False)

    barrel_broken = render(core, rom, {40: (DOWN,), 42: (RIGHT,), 46: (A,),
                                       63: (A,), 107: (A,), 112: (RIGHT,)})
    assert read_status(barrel_broken) == "D20 11 +4 = 15 DC14 BROKE"
    check_fighter(barrel_broken, 11, 5, "15", False)

    # Breaking one object leaves the others intact and available on later turns.
    two_objects = render(core, rom, crate | {108: (DOWN,), 112: (B,), 116: (UP,),
                                              120: (RIGHT,), 124: (A,), 129: (A,), 173: (A,)})
    assert re.fullmatch(r"D20 \d{2} \+4 = \d{2} DC14 (BROKE|FAIL)", read_status(two_objects))
    targets = {40: (DOWN,), 42: (RIGHT,), 46: (A,)}
    barrel_selected = render(core, rom, targets)
    crate_selected = render(core, rom, targets | {50: (DOWN,)})
    def bracket(frame, x, y):
        offset = 3 * (y * WIDTH + x)
        return tuple(v >> 3 for v in frame[offset:offset + 3]) == (31, 30, 18)
    assert bracket(barrel_selected, 176, 80)
    assert bracket(crate_selected, 144, 96)
    assert not bracket(crate_selected, 176, 80)
    assert render(core, rom, targets | {50: (DOWN,), 54: (UP,)}) == barrel_selected
    cycled_smash = render(core, rom, targets | {50: (DOWN,), 59: (A,), 103: (A,)})
    assert read_status(cycled_smash) == "D20 08 +4 = 12 DC10 BROKE"
    print("Crate and barrel OK. Distinct DCs, independent state and collision verified.")


def check_movement(core, rom, direct_rom, screenshot):
    enter = {30: (A,), 40: (START,)}
    starting_view = render(core, rom, enter)
    check_fighter(starting_view, 9, 4, "00", False)
    direct_view = render(core, direct_rom)
    assert direct_view == starting_view, "Direct ROM should boot into the trial"
    write_png(screenshot.with_name("river-trial.png"), direct_view)

    # Each tuple is one press, with simultaneous directions for diagonals.
    crossing = [(RIGHT,)] * 4
    scenarios = (
        ("swim one square", [(RIGHT,)], (10, 4, "10", False, True)),
        ("ordinary swimmer stops at 30 feet", crossing, (12, 4, "30", False, True)),
        ("Swim Speed halves calm-water cost", [(SELECT,)] + crossing, (13, 4, "20", True, True)),
        ("swim onto the bridge", [(RIGHT,), (DOWN,)], (10, 5, "15", False)),
        ("swim back to shore", [(RIGHT,), (LEFT,)], (9, 4, "15", False)),
        ("walk onto the bridge", [(DOWN,), (RIGHT,)], (10, 5, "10", False)),
        ("new turn keeps position", crossing + [(B,)], (12, 4, "00", False, True)),
        ("new turn allows movement again", crossing + [(B,), (RIGHT,)], (13, 4, "10", False, True)),
        ("turn off Swim Speed", [(SELECT,), (RIGHT,), (SELECT,), (RIGHT,)],
         (11, 4, "10", False, True)),
        ("wall blocks entry", [(LEFT,), (LEFT,), (UP, LEFT)], (7, 4, "10", False)),
        ("wall blocks diagonal corner", [(LEFT,), (LEFT,), (UP,), (DOWN, LEFT)],
         (7, 3, "15", False)),
        ("pine blocks entry", [(DOWN, LEFT)] * 4, (6, 7, "15", False)),
        ("marsh costs 10 feet", [(LEFT,)] * 5 + [(UP,), (B,), (UP,), (UP,)],
         (4, 1, "15", False)),
    )
    for name, presses, expected in scenarios:
        inputs = {50 + 4 * index: buttons for index, buttons in enumerate(presses)}
        frame = render(core, rom, enter | inputs)
        try:
            check_fighter(frame, *expected)
        except AssertionError as error:
            raise AssertionError(f"Movement scenario failed: {name}, expected {expected}") from error
        if name == "swim one square":
            write_png(screenshot.with_name("fighter-swimming.png"), frame)

    back_to_browsing = render(core, rom, enter | {50: (RIGHT,), 60: (START,)})
    assert back_to_browsing == render(core, rom, {30: (A,)})
    reentered = render(core, rom, enter | {50: (RIGHT,), 60: (START,), 70: (START,)})
    assert reentered == starting_view, "Reentering should restore the trial's starting state"
    check_animation(core, rom, enter)
    check_camera_animation(core, direct_rom)
    check_fence(core, direct_rom, screenshot)
    check_other_breakables(core, direct_rom)
    print(f"River Crossing OK. Idle, walking, swimming and camera animation verified. {len(scenarios)} movement scenarios, trial reset and direct boot verified.")


if __name__ == "__main__":
    core, rom, direct_rom, screenshot = map(Path, sys.argv[1:])
    screenshot.unlink(missing_ok=True)
    start = render(core, rom)
    check_cursor(start, 3, 5)
    write_png(screenshot, start)

    # A short hold moves once. A longer hold starts auto-repeat.
    held = {frame: (RIGHT,) for frame in range(30, 45)}
    moved = render(core, rom, held | {52: (DOWN,)})
    check_cursor(moved, 4, 6)
    check_terrain(start, moved, (3, 5), (4, 6))

    repeated = render(core, rom, {frame: (RIGHT,) for frame in range(30, 55)})
    check_cursor(repeated, 6, 5)

    top_left = render(core, rom, {frame: (LEFT, UP) for frame in range(30, 60, 2)})
    check_cursor(top_left, 0, 0)
    check_terrain(start, top_left, (3, 5), (0, 0))

    # These two stops put the cursor at the same screen position, with camera
    # origins one square apart on both axes. Compare their overlapping terrain.
    diagonal = {frame: (RIGHT, DOWN) for frame in range(30, 74, 2)}
    scrolled = render(core, rom, diagonal)
    shifted = render(core, rom, diagonal | {74: (RIGHT, DOWN)})
    check_cursor(scrolled, 13, 8)
    check_cursor(shifted, 13, 8)
    for y in range(HEIGHT - 16):
        for x in range(WIDTH - 16):
            if (x // 16, y // 16) in ((12, 7), (13, 8)):
                continue
            old = 3 * ((y + 16) * WIDTH + x + 16)
            new = 3 * (y * WIDTH + x)
            assert scrolled[old:old + 3] == shifted[new:new + 3], "Camera did not scroll 16 pixels"
    assert scrolled != shifted, "Camera did not reveal new terrain"

    map_source = Path(__file__).resolve().parents[1] / "src/battlefield-maps.h"
    rows = re.findall(r'"([.=#~T,:+]{64})"', map_source.read_text())
    assert len(rows) == 3 * 64, "Expected three 64×64 terrain layouts"
    far = {frame: (RIGHT, DOWN) for frame in range(60, 200, 2)}
    river = render(core, rom, {30: (A,)})
    keep = render(core, rom, {30: (A,), 40: (A,)})
    for index, (name, frame, switch) in enumerate((
        ("battlefield", start, {}),
        ("river-crossing", river, {30: (A,)}),
        ("old-keep", keep, {30: (A,), 40: (A,)}),
    )):
        terrain = rows[index * 64:(index + 1) * 64]
        check_cursor(frame, 3, 5)
        check_view(frame, terrain, 0, 0)
        write_png(screenshot.with_name(f"{name}.png"), frame)
        bottom_right = render(core, rom, switch | far)
        check_cursor(bottom_right, 14, 9)
        check_view(bottom_right, terrain, 49, 54)
        write_png(screenshot.with_name(f"{name}-far.png"), bottom_right)

    reverse = {frame: (LEFT, UP) for frame in range(210, 350, 2)}
    returned = render(core, rom, far | reverse)
    check_cursor(returned, 0, 0)
    check_view(returned, rows[:64], 0, 0)
    assert returned == top_left, "Returning after cache wrap should restore the original view"

    assert len({start, river, keep}) == 3, "Expected three distinct battlefields"
    assert render(core, rom, {30: (A,), 40: (A,), 50: (A,)}) == start
    reset = render(core, rom, diagonal | {84: (A,)})
    assert reset == river, "Map switch should reset cursor and camera"

    check_movement(core, rom, direct_rom, screenshot)

    print(f"mGBA battlefield OK. Fighter movement and HUD verified. Cache wrap, held input and all three 64×64 maps verified. Screenshot at {screenshot}")
