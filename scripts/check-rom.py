"""Check the cartridge header and the ARM branch into the startup code."""

import sys
from pathlib import Path


def check(rom):
    assert 0xC4 <= len(rom) <= 32 * 1024 * 1024, "Invalid ROM length"
    assert int.from_bytes(rom[:4], "little") == 0xEA00002E, "Expected ARM B to 0xC0"
    assert rom[4:0xA0] == bytes(156), "Expected the emulator-only blank logo field"
    assert rom[0xA0:0xB2] == b"GBA HELLO   AHLE00", "Unexpected cartridge identity"
    assert rom[0xB2:0xBD] == b"\x96" + bytes(10), "Invalid fixed or reserved fields"
    assert rom[0xBD] == (-sum(rom[0xA0:0xBD]) - 0x19) & 0xFF, "Bad header checksum"
    assert rom[0xBE:0xC0] == bytes(2), "Invalid trailing reserved bytes"


if __name__ == "__main__":
    rom = Path(sys.argv[1]).read_bytes()
    check(rom)
    print(f"Cartridge header OK ({len(rom)} bytes)")
