---
name: gba-development
description: Build and verify the tactical RPG's Game Boy Advance ROM on Linux or budget its tile, palette, sprite and display resources for original GBA hardware.
---

# GBA development

- Read the repository README for the pinned Linux bootstrap and tool overrides. Run `make test` from the repository root to validate the ROM header and its actual mGBA framebuffer.
- For startup changes, check the ELF map and the entry branch to ARM code. Update startup and linker together when adding initialized or zeroed writable globals. The current ROM uses the emulator boot path with a blank cartridge logo field.
- For rendering and asset changes, read `docs/gba-graphics.md`. Record mode, character/map ranges, palette banks, OAM slots, worst-case OBJ scanline cost and VBlank upload size for the busiest scene. Review output at 240×160 and distinguish emulator checks from measurements on a physical GBA.
- For visual direction, read `docs/shining-force-art.md`. Study map and close-up compositions and author new project graphics independently.
