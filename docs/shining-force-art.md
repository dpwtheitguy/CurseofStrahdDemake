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

Screenshots and manual pages are linked for study. Project art, fonts, maps and audio are authored separately. The ROM build reads only the source files in `src/`.

The breakable-object selector borrows the separation between a battlefield target and a highlighted command. River Crossing uses gold corner brackets on the target and three small illustrated cards. The upper card shows the selected object and its Athletics DC. The lower cards show Smash and Back, with a brighter gilt frame around the active command. [SEGA Shining Force II manual](https://www.sega.jp/genesismini2/assets/manual/pdf/US_Shining-Force-II.pdf) · [Shining Force II combat menu screenshot](https://infinityretro.com/shining-force-ii-review/)
