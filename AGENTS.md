# GBA project guidance

- Use [the GBA development skill](.agents/skills/gba-development/SKILL.md) when changing the ROM, build tooling or graphics.
- Use [the GBA terrain skill](.agents/skills/gba-terrain/SKILL.md) when adding or revising battlefield terrain. Each terrain type needs its own action close-up backdrop.
- Use [the GBA character equipment skill](.agents/skills/gba-character-equipment/SKILL.md) when changing loadouts, weapon actions, or equipment-driven sprites.
- Keep the Hello World build runnable through `make` and `make test`. Inspect `build/hello.png` after intentional boot-screen changes before updating the expected framebuffer hash.
- Keep reference material as documentation links. Use original project assets in the ROM.
