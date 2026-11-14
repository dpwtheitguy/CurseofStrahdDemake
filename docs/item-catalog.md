# Item catalog

The versioned [SRD catalog](../data/items-srd-5.2.1.json) currently contains 482 records. These include the core weapon and armor tables, ammunition, tools, adventuring gear, mounts, vehicles, poisons, and 258 magic-item heading records. Magic entries index the source and summarize classification. Their effects, prerequisites, charges, and per-variant behavior still need individual implementation. The catalog covers SRD content in the listed categories and does not claim coverage of every published D&D book.

The [authoring schema](../data/item-schema.json) separates common item metadata from weapon, armor, and magic data. IDs identify item definitions. Ammunition references a separate definition, and live ammunition counts and breakable HP live in game state. Prices are in copper. Range is in feet. Source pages refer to the English SRD PDF. The build checks duplicate IDs, ranges and references, then emits only the IDs and parameters consumed by the current ROM. Extend the generator when an item gains an implemented rule or pose.

## Ranged weapons and thrown options

| Group | Weapons |
| --- | --- |
| Simple ranged | Dart, light crossbow, shortbow, sling |
| Martial ranged | Blowgun, hand crossbow, heavy crossbow, longbow, musket, pistol |
| Melee weapons with Thrown | Dagger, handaxe, javelin, light hammer, spear, trident |

Dart already appears under simple ranged. The net, acid, and alchemist’s fire are gear with special attack replacements. Their rules need their own handlers.

## Try the longbow

At River Crossing, press **Enter**, then **Right** to equip the longbow and remove the shield. The top strip shows the arrow count. Press **Z**, then **X** to open commands. Use **Up/Down** to select a visible fence, crate, or barrel, then select **Action → Shoot**. Inspect the character panel again to see arrows remaining. **Backspace** in that panel returns to basic gear switching.

The trial models a clear tile-center path, normal and long range, ammunition, hit rolls, criticals, and object HP. The object hit points are trial values. Creature cover, enemy turns, and weapon mastery effects are outside this object trial.

This work includes material from the System Reference Document 5.2.1 (“SRD 5.2.1”) by Wizards of the Coast LLC, available at https://www.dndbeyond.com/srd. The SRD 5.2.1 is licensed under the Creative Commons Attribution 4.0 International License, available at https://creativecommons.org/licenses/by/4.0/legalcode. The source material was adapted into structured game data.
