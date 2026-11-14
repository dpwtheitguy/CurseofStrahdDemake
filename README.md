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

From Ruined Road, press **X** once for River Crossing, then **Enter** to start the trial. The arrow keys move the fighter. The top strip shows movement spent out of 30 feet. Swim Speed appears in the character panel, and Athletics rolls appear in the close-up. **Z** starts a fresh turn at the current square. **Backspace** toggles a 30-foot Swim Speed and resets the movement counter. **Enter** opens or closes Alden’s character panel. **Z** also closes it. Hold **A** (GBA L) and press **Enter** to return to map browsing.

Each square represents 5 feet. A bridge step costs 5 feet, marsh costs 10, and calm water costs 10 or 5 depending on Swim Speed. Walls and pines block movement. The trial spends movement step by step and rejects steps beyond the budget. The sprite and font are in [battlefield-fighter.h](src/battlefield-fighter.h), and `step_cost` in [battlefield.c](src/battlefield.c) applies the terrain rules.

Press **X** to open **Action**, **Movement** (Move), **Bonus Action** (Bonus), **Items**, and **Cancel**. Use **Left/Right** to choose a category. Use **Up/Down** to cycle nearby targets. Gold brackets mark the target, and a blue side panel shows its name and type. Press **Backspace** (GBA Select) to open **Details** and **Z** to return to the same command. Press **X** to confirm a command or **Z** to go back.

**Action → Smash** spends the Action on the selected breakable. **Movement → Move** shows the feet left this turn and returns to D-pad movement. **Bonus Action → Dash** adds movement when the Bonus Action is available. The object's DC appears in the roll close-up.

Select **Action → Dash** to spend the regular Action, or **Bonus → Dash** to use the trial’s separate Bonus Action. Either adds Alden’s Speed to this turn’s movement budget. The trial grants Alden a Bonus Action Dash, separate from the Action used for Smash. Already-spent movement and terrain costs still apply. **Z** outside the menu starts a new turn and restores the normal budget and both action resources. [2024 Dash rule](https://www.dndbeyond.com/sources/dnd/br-2024/rules-glossary#Dash)

Smash uses a Strength (Athletics) check of **d20 + 4**. The crate is DC 10, the fence DC 12, and the barrel DC 14. Each attempt spends an action and plays a close-up with the roll breakdown. Press **X** or **Z** after the result to return to the map. Success leaves passable splinters. The [breakable object reference](docs/breakable-objects.md) records BG3 examples and encounter ideas.

The character panel shows the fighter’s name, class, HP, AC, Speed and Athletics bonus. The movement budget and Smash check use the Speed and Athletics values from the same character record.

The fighter breathes when idle and slides between grid squares. When the camera follows, terrain, props, and target brackets use the same pixel scroll offset. Build the direct-boot version with `make build/river-trial.gba`.

### Action close-up preview

Smash opens with Alden bracing on the near right. He lunges toward the target on the far left, then recoils. The scene chooses its backdrop from the actor and target squares. A barrel on the bridge shows planks over the river. A fence on dry ground shows a field. Grass, dirt road, ruined wall, river, pine forest, marsh, cobblestone, and bridge each have their own backdrop. The higher-priority terrain wins when actor and target stand on different surfaces. See the [terrain definitions](src/battlefield-terrain.def) and [terrain authoring skill](.agents/skills/gba-terrain/SKILL.md). A successful hit throws splinters and leaves debris. A failed hit leaves the object standing. Snow art is ready for a future snow square.

For a private music preview, point the optional target at a local 16-bit PCM WAV. For example, use the Shining Force standard battle scene cue from your local reference folder.

```sh
make preview-battle-audio BATTLE_WAV="/absolute/path/to/battle-cue.wav" MAP_WAV="/absolute/path/to/map-cue.wav"
mgba build/preview-local-only.gba
```

With MAP_WAV supplied, this separate ROM loops a 30-second map excerpt and restarts it when you return from Smash. Smash plays a 30-second battle excerpt. The clips ease in and out, and changing scenes tapers the outgoing cue. Keep this preview ROM local. The normal ROM targets use original project art and omit the local WAV.

Alden has a warhammer, chain shirt, and shield. In the character panel, press **Backspace** (GBA Select) to toggle between that starting kit and the unarmed, unarmored loadout with basic clothing. Press **Z** to return to the map and try Smash. Both views use the same loadout, and the panel shows AC 16 with gear and AC 11 without it. The toggle is for testing equipment visuals and AC. Smash still resolves as the trial's Athletics check.

See the [item catalog and longbow controls](docs/item-catalog.md) for the 2024 equipment data and ranged trial.

**Z** on the map ends the turn with a short click and a centered **NEW TURN** banner that sweeps off to the right. River Crossing hands control back to Alden immediately.

Use **Items** on the main command row for equipment changes. From Action, press Left twice to reach Items. In the blue list, use Up/Down and press X to confirm. Drawing and stowing happen in place. The first ordinary interaction uses the turn's free interaction. A second spends the Action on Utilize if available. Doffing or donning a shield always uses Utilize. The panel shortcuts remain available for playtesting. [Object interactions](https://www.dndbeyond.com/sources/dnd/br-2024/playing-the-game#YourTurn) · [Shield rules](https://www.dndbeyond.com/sources/dnd/br-2024/equipment#Armor)

Run `make unit-test` for native equipment and target-selection rules without mGBA. After rebuilding the private music ROM, run `make smoke-preview` to drive Items, verify the persistent bow sprite and target cycling, and fire at the barrel in headless mGBA.

For a faster bow switch, choose **Items → Longbow** from the starting kit. **STOW AND DOFF** uses the free interaction to stow the hammer and the Action to doff the shield. End the turn, then open **Action** and confirm **Draw and Shoot**. With the shield already off, Draw and Shoot can stow a held hammer using an available free interaction and draw the bow as part of the Attack in the same turn.

The Items list keeps Hammer, Longbow, and Shield visible with ON or STOWED status. Selecting Hammer while holding the bow can stow the bow with the free interaction and draw the hammer with Utilize in one confirmation when both are available.

River Crossing has a wooden training dummy and two archery targets just beyond the bridge. The battle close-up slides Alden in from the right and the target in from the left over about a quarter second. Press **X** during the animation to jump to the result, then **X** or **Z** to return.
