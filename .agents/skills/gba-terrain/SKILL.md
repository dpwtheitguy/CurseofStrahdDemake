---
name: gba-terrain
description: Add or revise terrain in the tactical RPG, pairing each battlefield tile with original action close-up scenery and verifying map behavior and GBA rendering.
---

# GBA terrain

Work from the project root and use [GBA development](../gba-development/SKILL.md) for ROM builds and graphics budgets.

1. Define the terrain symbol and its 16×16 map art in [battlefield.c](../../../src/battlefield.c) and [battlefield-terrain.def](../../../src/battlefield-terrain.def). A square is 5 feet and uses four 8×8 hardware tiles. Check palette and VRAM ranges before reserving tiles. Check movement cost, passability, and traversal poses against the terrain's intended rules before placing it in [the maps](../../../src/battlefield-maps.h).
2. Give the symbol its own backdrop in [generate-closeup.py](../../../scripts/generate-closeup.py). Author a distinct ground surface and recognizable features with original pixels. Use the 240×112 scenery band and 15 visible colors in the existing 4bpp palette. Keep the target's footing high-left, Alden's footing low-right, and the area behind their silhouettes readable. Check both positions against the terrain at native resolution.
3. Set the backdrop priority in the terrain definition. The higher priority wins across mixed squares, with the actor winning ties. An elevated bridge deck takes precedence over river water. Check a mixed pair when the new terrain touches another surface.
4. Run `make test`. The art generator rejects unregistered map symbols, missing backdrop art, and shared backdrop assignments. Verify the new map tile and a close-up screenshot. Exercise the action in mGBA when that terrain has a reachable target. Record any art-only review, and update [the graphics budget](../../../docs/gba-graphics.md) if resource use changes.
