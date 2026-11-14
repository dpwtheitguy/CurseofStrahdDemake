# Shining Force art references

These are the Genesis/Mega Drive releases. Open the linked frames alongside the project at integer zoom when comparing pixels. The observations below describe composition and readability for original artwork.

| Reference | What to study |
| --- | --- |
| Shining Force, [town exterior](https://www.asteroidg.com/images/screenshots/shining_force_02.gif) | Frontal walls and overhead paths share the same view. Trees and flowerbeds frame the route. Roofs, doors and columns make buildings readable through large shapes. |
| Shining Force, [village battle](https://www.player.one/shining-force-best-jrpg-youve-never-played-526841) and [dungeon battle](https://fantasyanime.com/shiningforce/screenshots/large/sf1shot103.png) | Small units sit against connected grass, fences or stairs. The terrain still reads beneath the units, with compact status and terrain panels at the edges. The dungeon uses stairs and visible wall faces to suggest height. |
| Shining Force, [Ramladu combat close-up](https://fantasyanime.com/shiningforce/screenshots/large/sf1shot97.png) | Opposing silhouettes occupy separate sides of a torch-lit corridor. The floor anchors the figures while darker architectural detail recedes behind them. |
| Shining Force II, [castle exploration](https://www.mobygames.com/game/7027/shining-force-ii/screenshots/genesis/33465/) and [river battle](https://www.bojoga.com.br/files/2010/02/gfs_45479_2_16.jpg) | The river bends around open ground and dark forest masses. White and colored clothing help separate adjacent units from the grass. The split HUD leaves the center available for the cursor and units. |
| Shining Force II, [Slade attack close-up](https://www.mobygames.com/game/7027/shining-force-ii/screenshots/genesis/604000/) | The cut to larger figures makes the individual attack legible. Study the scene framing, where the foreground fighters sit, and how the background establishes the location. |

The [SEGA Shining Force manual](https://shared.fastly.steamstatic.com/store_item_assets/steam/apps/71120/manuals/02%20SF_PC_MG_EFIGS_US_v6.pdf) includes exploration and battle examples. The [SEGA Shining Force II manual](https://www.retrogames.cz/manualy/Genesis/Shining_Force_II_-_Genesis_-_Manual.pdf#page=8) shows the movement-area preview, terrain readout, unit panel and post-move command flow.

## Direction for the GBA prototype

- Try 16×16 logical terrain cells assembled from four 8×8 hardware tiles. A tile-aligned 240×160 viewport fits 15×10 cells before HUD occlusion. Keep the overhead grid implicit in terrain seams and show a cursor and reach overlay on demand.
- For an original Barovian map, start with a muddy road, a dark treeline and pale stone walls. Group terrain into broad shapes. Give the cursor and unit silhouettes higher contrast than environmental texture. Reserve a separate HUD layer for the active unit, terrain and current command.
- Prototype close-ups as their own scene with a ground plane, a restrained backdrop and two readable poses. Load scene-specific tiles and palettes during the transition. Compare walkable terrain, allied and enemy silhouettes, and text at native resolution before adding details.
- Keep rules detail in context panels. BG3 and D&D rules will need room for action economy, statuses and targeting beyond the few stats these screenshots display.

Screenshots and manual pages are linked for study. Project art, fonts, maps and audio are authored separately. The standard ROM uses original project assets and generated scenery. An opt-in private build can audition a locally supplied music cue.

The breakable-object selector borrows the separation between a battlefield target and a highlighted command. River Crossing uses gold corner brackets on the target and a compact blue panel on the opposite side of the map. The panel shows the object name and opens a Details view with Select. Illustrated cards along the bottom show the active menu level, with a brighter gilt frame around the focused command. Roll details appear in the close-up. [SEGA Shining Force II manual](https://www.sega.jp/genesismini2/assets/manual/pdf/US_Shining-Force-II.pdf) · [Shining Force II combat menu screenshot](https://infinityretro.com/shining-force-ii-review/)

## Character status on a handheld screen

- The Genesis games keep short status readouts near the edge during battle. Shining Force II puts the detailed party view behind the Member command and lets the player flip between status screens. [Shining Force battle screenshot](https://fantasyanime.com/shiningforce/screenshots/large/sf1shot103.png) · [SEGA Shining Force II manual](https://cdn.cloudflare.steamstatic.com/steam/apps/71160/manuals/09%20SF2_PC_MG_EFIGS_US_v6.pdf)
- The GBA remake places small status windows around the battlefield. The foreground units and routes remain readable even with a target and its stats on screen. Use that footprint as the handheld reference. [Atlus GBA release and screenshot gallery](https://atlus.com/atlus-titles/sf_res/?ItemID=111) · [GBA battle screenshot](https://thekingofgrabs.com/2022/12/09/shining-force-resurrection-of-the-dark-dragon-game-boy-advance/shining-force-resurrection-of-the-dark-dragon-gba-049/)
- Alden’s panel occupies 128×72 pixels and appears on demand. An original 32×32 portrait and gold name establish identity. The class label and aligned stat rows use one muted frame and a shared type style. The map remains visible around the panel, and the object command cards and character panel use separate interaction states.

The Smash camera places Alden low on the right, catching the light on his helmet and shield, with the object higher on the left. The two ground contacts, smaller target, distant skyline, and diagonal lunge establish depth. The blue and gold result frame sits below the art.

The Attack submenu gives eligible squares a gentle brightness pulse that preserves the terrain texture. Hammer and bow share the effect, and the selected target keeps steady gold brackets. Drawing the hammer also briefly pulses its reach while the equipment confirmation is visible. The combat options spread describes a flashing striking-distance grid and separate target brackets. [SEGA Shining Force II manual, printed pages 14–15](https://www.sega.jp/genesismini2/assets/manual/pdf/US_Shining-Force-II.pdf#page=9)
