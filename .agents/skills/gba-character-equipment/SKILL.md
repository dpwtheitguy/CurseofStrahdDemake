---
name: gba-character-equipment
description: Add or revise tactical RPG character loadouts, weapon actions, armor, or equipment-driven art on the GBA battlefield and in action close-ups.
---

# GBA character equipment

Use [GBA development](../gba-development/SKILL.md) when editing ROM code or graphics.

- Keep equipped weapon, armor, and shield on the character record in [battlefield-character.h](../../../src/battlefield-character.h). Use explicit unarmed, unarmored, and empty-offhand states. Give those states valid AC, movement poses, and close-up art. Basic cloth can cover an unarmored character without granting an armor bonus.
- Use the loadout and capabilities to expose legal actions. Keep item rules such as range, damage, hands required, and mastery alongside the item definition when implementing them. Keep a resolved action distinct from the item so a handaxe can strike or be thrown. Shove and grapple are Unarmed Strike options under the 2024 rules. Use the [2024 equipment rules](https://www.dndbeyond.com/sources/dnd/br-2024/equipment) and [Rules Glossary](https://www.dndbeyond.com/sources/dnd/br-2024/rules-glossary).
- Update both the 16×16 map poses in [battlefield.c](../../../src/battlefield.c) and the close-up poses in [generate-closeup.py](../../../scripts/generate-closeup.py). At map scale prioritize silhouette and keep swimming readable. In close-up align weapon and shield to animated hands, and show the equipment leaving the hand for a throw. Revisit the pose storage strategy before multiplying large sets of weapon, armor, and shield combinations.
- Verify an equipped and unequipped character through the normal game controls. Check AC, walk and idle, swimming if affected, and the same action on the same terrain. The River Crossing character panel currently has a prototype gear toggle for that comparison. Run `make test` and inspect the emulator output at native resolution.
