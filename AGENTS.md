# GBA project guidance

- Use [the GBA development skill](.agents/skills/gba-development/SKILL.md) when changing the ROM, build tooling or graphics.
- Keep the Hello World build runnable through `make` and `make test`. Inspect `build/hello.png` after intentional boot-screen changes before updating the expected framebuffer hash.
- Keep reference material as documentation links. Use original project assets in the ROM.
