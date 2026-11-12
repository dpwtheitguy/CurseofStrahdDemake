# GBA tactical RPG · Hello World

The first milestone builds a freestanding ARM7TDMI ROM and verifies that mGBA renders an original 240×160 “HELLO WORLD” screen. The Shining Force reference frames and their implications for the tactical map are in [Art direction](docs/shining-force-art.md). The display limits and budgeting workflow are in [GBA graphics](docs/gba-graphics.md).

## Build on Linux

On Ubuntu 24.04 or Pop!_OS 24.04 amd64 with the Ubuntu Universe apt repository enabled, use Bash, Make, Python 3, apt, dpkg and coreutils.

```sh
./scripts/bootstrap-linux.sh
make
make test
```

Bootstrap downloads the exact GCC 13.2.Rel1, binutils 2.42 and mGBA 0.10.2 Ubuntu packages and the core's libinih dependency. It extracts them under `.toolchain/` without root. After setup the build and smoke test run offline. If an exact revision leaves your apt mirror, use an Ubuntu archive retaining those revisions or supply system tools. With system tools, run `make test CROSS_COMPILE=arm-none-eabi- MGBA_CORE=/absolute/path/to/mgba_libretro.so`. The compiler and core must run on your Linux host.

- `make` emits `build/hello.gba`, `build/hello.elf` and `build/hello.map`.
- `make test` checks the entry branch and cartridge header, runs 120 frames with the mGBA libretro core, and compares the final framebuffer against the reviewed screen. It writes `build/hello.png` for inspection. The smoke test uses mGBA's built-in BIOS emulation and needs no GUI or BIOS file.
- `make run MGBA=/path/to/mgba` opens the ROM in a separately installed desktop frontend. The Ubuntu SDL package installs its executable at `/usr/games/mgba`. You can also open the ROM from mGBA's menu.
- `make clean` removes generated build outputs. After switching compilers, clean before rebuilding.

The default ROM has a blank cartridge logo field for emulator boot. A stock GBA BIOS expects the official cartridge logo bytes, so this artifact targets mGBA and has no physical cartridge boot validation. See [GBATEK's cartridge header reference](https://mgba-emu.github.io/gbatek/#gba-cartridge-header).

Startup sets the stack and calls freestanding C. The linker rejects writable globals until startup implements data copying and BSS clearing. Project code and the hand-drawn glyphs are original. Shining Force material appears only as external links in the art notes.

## Battlefield prototype

Run `make battlefield` to build `build/battlefield.gba`. Open it in mGBA or run `make run-battlefield MGBA=/path/to/mgba`.

- Use the D-pad or arrow keys to move the gold cursor. Hold a direction to repeat. The camera follows across a 64×64 map. Each square is 5 feet, so each battlefield covers 320×320 feet.
- Press GBA A (X in the desktop emulator) to cycle Ruined Road, River Crossing and Old Keep. Switching maps resets the cursor and camera.

Edit [the map layouts](src/battlefield-maps.h) to change the terrain. In [battlefield.c](src/battlefield.c), `cache_square` turns each terrain character into four tile references. `stream_edges` loads newly visible squares. `follow` and `show_view` position the camera and cursor.

`make smoke-battlefield` saves screenshots of the starting view and far corner of all three maps. It tests held input, camera scrolling, tile cache wrap, map bounds and cycling. `make test` runs both ROMs' smoke tests.

### River Crossing movement trial

From Ruined Road, press **X** once for River Crossing, then **Enter** to start the trial. The arrow keys move the fighter. The HUD shows movement spent out of 30 feet. **Z** starts a fresh turn at the current square. **Backspace** toggles a 30-foot Swim Speed and resets the movement counter. **Enter** returns to map browsing.

Each square represents 5 feet. A bridge step costs 5 feet, marsh costs 10, and calm water costs 10 or 5 depending on Swim Speed. Walls and pines block movement. The trial spends movement step by step and rejects steps beyond the budget. The sprite and font are in [battlefield-fighter.h](src/battlefield-fighter.h), and `step_cost` in [battlefield.c](src/battlefield.c) applies the terrain rules.

Press **X** beside the fence, crate, or barrel to select the nearest intact object. Gold brackets mark the target and three illustrated cards show the object, **Smash**, and **Back**. The gold frame marks the active command. Use **Up/Down** to cycle nearby intact targets and **Left/Right** to choose **Smash** or **Back**, press **X** to confirm, or **Z** to close the menu. Outside the menu, **Z** starts a fresh turn.

Smash uses a Strength (Athletics) check of **d20 + 4**. The crate is DC 10, the fence DC 12, and the barrel DC 14. Each attempt spends an action and plays a close-up with the roll breakdown. Press **X** or **Z** after the result to return to the map. Success leaves passable splinters. The [breakable object reference](docs/breakable-objects.md) records BG3 examples and encounter ideas.

The fighter breathes when idle and slides between grid squares. When the camera follows, terrain, props, and target brackets use the same pixel scroll offset. Build the direct-boot version with `make build/river-trial.gba`.
