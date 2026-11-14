"""Convert a local PCM WAV to signed mono samples for the preview ROM."""
import array
import sys
import wave
from pathlib import Path

RATE = 8192


def main():
    source, destination = map(Path, sys.argv[1:3])
    seconds = int(sys.argv[3]) if len(sys.argv) > 3 else 8
    with wave.open(str(source)) as wav:
        channels, rate, frames = wav.getnchannels(), wav.getframerate(), wav.getnframes()
        if wav.getsampwidth() != 2 or channels not in (1, 2) or rate < RATE:
            raise ValueError("Expected mono or stereo 16-bit PCM at 8192 Hz or higher")
        pcm = array.array("h", wav.readframes(frames))
    if sys.byteorder != "little":
        pcm.byteswap()
    count = min(frames * RATE // rate, seconds * RATE)
    output = bytearray()
    for i in range(count):
        start = i * rate // RATE * channels
        end = (i + 1) * rate // RATE * channels
        sample = sum(pcm[start:end]) // (end - start)
        fade = min(i + 1, count - i, RATE // 2)
        output.append((int(sample * fade / (512 * (RATE // 2)))) & 255)
    # Allow FIFO bursts beyond the final sample while waiting for the next frame.
    output.extend(bytes(seconds * RATE + 1024 - len(output)))
    destination.write_bytes(output)


if __name__ == "__main__":
    main()
