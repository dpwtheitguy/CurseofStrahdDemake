# GBA graphics constraints

| Hardware | Constraint and implication |
| --- | --- |
| Display and timing | 240×160 at about 59.73 Hz. Each line lasts 1,232 CPU cycles and each frame 280,896. Lines 160–227 are outside visible drawing, roughly 83,776 cycles including the final line. Budget uploads and OAM commits in VBlank and measure the worst case. [Tonc video timing](https://gbadev.net/tonc/video.html) |
| VRAM | 96 KiB total. Tile modes allocate 64 KiB for BG character/map data and 32 KiB for OBJ tiles. Bitmap modes leave 16 KiB for OBJ tiles. Regular BG character blocks step by 16 KiB and screen blocks by 2 KiB within shared BG VRAM. Explicitly check for overlaps. [GBATEK display control and OBJ memory](https://mgba-emu.github.io/gbatek/#lcd-io-display-control) |
| Tile and map data | Tiles are 8×8. A 4bpp tile costs 32 bytes and an 8bpp tile costs 64. A regular BG entry is 16 bits with a 10-bit tile index, flip bits and a palette bank field. A 32×32 screen block costs 2 KiB. Regular BG sizes are 256 or 512 pixels on each axis, with screen blocks arranged by size. [Tonc tiled backgrounds](https://gbadev.net/tonc/regbg.html) |
| Palette | Colors use 5 bits per channel. BG and OBJ have separate 512-byte palettes, each with 256 entries. 4bpp uses sixteen banks of sixteen entries, with pixel index zero transparent in each bank. 8bpp indexes the full palette and uses zero for transparency in tiled BGs and OBJs. BG palette entry zero supplies the backdrop. [Tonc palettes](https://gbadev.net/tonc/video.html) |
| Layers | Mode 0 has four regular tiled BGs. Mode 1 has two regular BGs and one affine BG. Mode 2 has two affine BGs. Modes 3–5 use bitmap BG2. Mode 3 is single-buffered 240×160 direct color (75 KiB), Mode 4 is 240×160 indexed with two pages, and Mode 5 is 160×128 direct color with two pages. [GBATEK video modes](https://mgba-emu.github.io/gbatek/#lcd-io-display-control) |
| Sprites | OAM holds 128 entries and 32 affine matrices in 1 KiB. Legal sprite shapes range from 8×8 to 64×64 with specific rectangular sizes. Scanline cost matters independently of OAM count. GBATEK lists 1,210 cycles per scanline for OBJ evaluation, dropping to 954 with HBlank Interval Free enabled. Normal sprites cost one cycle per pixel of width. Affine sprites cost ten plus twice the rendered width, including the doubled bounding box if enabled. [GBATEK OBJ overview](https://mgba-emu.github.io/gbatek/#lcd-obj---overview) |
| Write access | Use aligned 16- or 32-bit writes for VRAM, palette RAM and OAM. Byte stores can duplicate bytes in some video regions or be ignored. Commit changing OAM during VBlank or initialize in forced blank. [GBATEK video memory](https://mgba-emu.github.io/gbatek/#lcd-vram-overview) |

## Rendering baseline

Hello World initializes a static Mode 3 screen in forced blank. The battlefield prototype uses Mode 0 with a terrain background and OBJ cursor. A reach overlay and HUD can each use a separate background as they are added. Use 4bpp first and record tile/map ranges, palette bank assignments, OAM count, worst scanline and bytes transferred per frame when an asset set changes. For example, 128 unique 4bpp terrain tiles occupy 4 KiB and one 32×32 regular map occupies another 2 KiB. Keep the scanline calculation tied to the busiest reachable scene, including overlapping effects and oversized sprites.

The smoke test measures emulator output. Hardware review should include native-resolution text, contrast on the intended LCD, worst-case sprite overlap and VBlank timing on the intended flashcart and console.

## Battlefield prototype budget

| Resource | Allocation |
| --- | --- |
| BG character data | Block 0 at `0x06000000–0x060003FF`, thirty-two 4bpp tiles (1 KiB) |
| BG map | Four screen blocks (16–19) at `0x06008000–0x06009FFF` (8 KiB). A 32×32-square wrapping cache for a 64×64-square world. The viewport shows 30×20 hardware tiles. |
| OBJ character data | Four 4bpp tiles (128 bytes) at `0x06010000–0x0601007F` in 1D mapping |
| Palettes | BG bank 0 uses fifteen entries including backdrop. BG bank 1 adds twelve visible colors. OBJ bank 0 uses index zero for transparency and two visible colors. |
| OAM | One active 16×16 sprite, remaining 127 entries disabled. Worst visible scanline uses 16 OBJ rendering cycles. |
| Frame updates | Tiles and palettes load once in forced blank. Switching maps loads 1,200 bytes of visible map entries in forced blank. Camera movement loads one incoming row and/or column during VBlank. A diagonal update writes at most 25 squares at eight bytes each, plus eight bytes for BG scroll and OAM. |

These are calculated allocations for the current scene. The mGBA smoke test verifies rendering and input.

The movement trial uses BG1 for the HUD and command panel. Its 38 character tiles (1,216 bytes) occupy `0x06000400–0x060008BF`, with screen block 20 at `0x0600A000–0x0600A7FF`. BG palette bank 2 supplies the HUD, and banks 3 and 4 supply the bronze and gilt card colors. Five illustrated card variants use 80 tiles (2,560 bytes) at `0x06000A00–0x060013FF`. Three cards are visible at a time. A full redraw writes 48 map entries (96 bytes).

The fighter's standing, breathing, walking, and two swimming poses use 20 OBJ tiles (640 bytes) in palette bank 1. The three breakables and shared debris use 16 OBJ tiles (512 bytes) in bank 2. Fighter, three breakables, and target brackets use up to five OAM slots. The brackets reuse the cursor's four tiles. Five overlapping 16-pixel sprites would cost at most 80 OBJ cycles on a scanline.

In the trial, pixel interpolation updates BG scroll and matching OAM coordinates during VBlank. A movement frame can write 200 bytes of streamed terrain, 60 bytes of movement HUD, and up to 36 bytes of scroll and sprite attributes.

The Smash close-up uses BG1 screen block 21 (`0x0600A800–0x0600AFFF`), six scenery tiles at `0x06001400–0x060014BF`, and BG palette bank 5. The transition loads 600 visible map entries and four spark OBJ tiles during forced blank. Two 32×32 affine canvases reuse the normal fighter and object tile art at 2× scale. The impact adds a third canvas, for a worst-case 222 OBJ rendering cycles per scanline. The map's tile cache remains resident throughout the scene.
