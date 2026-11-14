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

The movement trial reserves the top eight screen pixels for the movement HUD, leaving nine complete rows of 5-foot squares and an eight-pixel preview of the next row. At the southern edge, the viewport aligns to the bottom of the map. BG1 draws the HUD and command panel. Its 40 character tiles (1,280 bytes) occupy `0x06000400–0x060008FF`, with screen block 20 at `0x0600A000–0x0600A7FF`. BG palette bank 2 supplies the HUD, and banks 3 and 4 supply the bronze and gilt card colors. Smash and Back use 32 tiles (1,024 bytes) at `0x06001000–0x060013FF`. Dash, Bonus, Action, Move, and Cancel use 80 tiles (2,560 bytes) at `0x06001900–0x060022FF`. Bank 9 supplies dimmed cards. The root shows five command cards. The Items card uses 16 tiles at `0x06000A00–0x06000BFF` and loads via DMA3 during startup forced blank. The 112×40 target panel uses banks 10–11 and shares the nine frame tiles at `0x06001500–0x0600161F` with the character panel. A target cycle can clear and redraw the panel, draw four cards, and update the notice in the same VBlank. That path writes at most 508 bytes of BG map entries and 8 bytes of target OAM.

The fighter's standing, breathing, walking, and two swimming poses use 20 OBJ tiles (640 bytes) in palette bank 1. The three breakables and shared debris use 16 OBJ tiles (512 bytes) in bank 2. Fighter, three breakables, and target brackets use up to five OAM slots. The brackets reuse the cursor's four tiles. Five overlapping 16-pixel sprites would cost at most 80 OBJ cycles on a scanline.

In the trial, pixel interpolation updates BG scroll and matching OAM coordinates during VBlank. A movement frame can write 200 bytes of streamed terrain, 60 bytes of movement HUD, and up to 36 bytes of scroll and sprite attributes.

The Mode 0 Smash close-up uses BG1 screen block 21 (`0x0600A800–0x0600AFFF`). A 240×112 scenery band uses 420 4bpp tiles at `0x06002800–0x06005C7F` and BG palette bank 5. Only the selected terrain backdrop occupies VRAM. Each additional backdrop adds 13,472 bytes of art and palette data in ROM. The dialog reuses the frame and font tiles in banks 12–13. The scene loads 13,440 bytes of scenery, 1,792 bytes of pose and effect tiles, 1,200 bytes of BG map entries, and 54 bytes of palette data in forced blank. The terrain cache remains resident. Alden uses one 64×64 affine canvas. The target and impact effect use two 32×32 affine canvases. If all overlap on one line, the maximum is 286 OBJ rendering cycles using the GBATEK affine formula of 10 + 2 × canvas width. The result redraw writes 68 bytes of BG map entries plus up to 18 bytes of OAM during VBlank. [GBATEK OBJ rendering](https://mgba-emu.github.io/gbatek/#lcd-obj-overview)

The opt-in local audio preview reserves up to 241 KiB of EWRAM for signed 8-bit mono PCM and padding. DMA3 stages the selected cue from ROM into a shared buffer. Timer 0 clocks FIFO A at 8,192 Hz and DMA1 reloads it from EWRAM in 16-byte bursts. Timer 1 counts samples for short fade-outs on scene changes. Clip boundaries have half-second fades baked into the samples. Audio timing was checked in mGBA. [GBATEK Direct Sound](https://mgba-emu.github.io/gbatek/#gba-sound-channel-a-and-b---dma-sound)

The character panel adds 25 tiles (800 bytes) at `0x06001500–0x0600181F` and uses BG palette banks 6–8. The 128×72 panel shares BG1 screen block 20 with the HUD and menu. It needs no additional OAM slots. The shared frame tiles load at startup. Portrait tiles load in forced blank on first inspection. Later openings update the panel's map entries during VBlank.

Equipment swaps regenerate the five 16×16 map poses (640 bytes of OBJ tile data) in forced blank. Each loadout combination adds 1,536 bytes of generated close-up pose art to ROM. Only the chosen combination is uploaded on scene entry. Equipment uses the existing fighter OBJ palette and sprite, so the scene keeps its previous OAM and scanline budget.

The bow uses the same close-up actor and effect slots. The arrow selects four additional OBJ tiles at `0x06010B80–0x06010BFF`. The Shoot card adds 512 bytes at `0x06000C00–0x06000DFF` and reuses the command palettes.

The Items submenu shares the blue panel frame, font, palette banks 10–11 and BG1 screen block 20. The 144×64 panel writes up to 386 bytes of BG map entries per draw and adds no OAM slots. The turn cue uses the GBA noise channel alongside Direct Sound music. [GBATEK noise channel](https://mgba-emu.github.io/gbatek/#gba-sound-channel-4---noise)

The Mode 0 turn banner uses two 64×32 sprites in OAM slots 5–6 and OBJ palette bank 4. Its 64 tiles occupy `0x06010C00–0x060113FF` and load during startup forced blank. It adds up to 128 OBJ rendering cycles per covered scanline, for a conservative total of 208 cycles with the map actors. Animation writes at most 12 bytes of OAM per VBlank. The banner clears when opening a menu, switching map views, or completing its 24-frame sweep.

Gear confirmation reuses the actor's lowered pose on land and adds one 16×16 item sprite in OAM slot 7. The three icons use 384 bytes at `0x06011400–0x0601157F` in OBJ palette bank 5, loaded in forced blank when applying equipment. The icon costs up to 16 OBJ cycles on a scanline and at most six bytes of OAM writes per frame. Ending a turn clears the gear cue before showing the turn banner.

The equipment confirmation uses row 19 of the existing BG1 HUD map for 84 frames. It reuses the HUD glyphs and palette, writing 60 bytes of map entries on show and clear.

The Attack submenu and hammer draw confirmation use Mode 0 BG2 to repeat eligible terrain squares between the terrain and HUD. Screen block 22 occupies `0x0600B000–0x0600B7FF`. It reuses the terrain tiles and palettes and adds no OAM usage. Opening or changing the overlay clears 2,048 map bytes and writes up to 1,408 bytes of square entries during forced blank. The hardware brightness effect targets BG2 with a coefficient from 1/16 to 3/16 over a 48-frame cycle. Each pulse frame updates one register during VBlank. [GBATEK color special effects](https://problemkaputt.de/gbatek.htm#lcdiocolorspecialeffects)

The bridge approach adds a pallet, basket, and stool. Their 12 OBJ tiles occupy `0x06011580–0x060116FF` and reuse object palette bank 2. They use OAM slots 8–10, adding up to 48 OBJ cycles per scanline and 18 bytes per map update. The close-up hides all additional breakable slots before drawing its actors.

The dummy and archery target add eight OBJ tiles at `0x06011700–0x060117FF`. Three practice props reuse the existing object palette and use OAM slots 11–13. They add up to 48 OBJ cycles per scanline and 18 bytes per map update. The close-up entrance changes the positions of its existing two sprites over 15 frames.
