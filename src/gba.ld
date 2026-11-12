ENTRY(_start)

MEMORY {
    rom (rx) : ORIGIN = 0x08000000, LENGTH = 32M
}

SECTIONS {
    .gba_header : { KEEP(*(.gba_header)) } > rom
    .text : { *(.text.boot) *(.text*) *(.rodata*) } > rom
    .data : { *(.data*) *(.sdata*) } > rom
    .bss (NOLOAD) : { *(.bss*) *(.sbss*) *(COMMON) } > rom
}

ASSERT(SIZEOF(.gba_header) == 0xc0, "GBA cartridge header must be 192 bytes")
ASSERT(boot == 0x080000c0, "Startup must immediately follow the header")
ASSERT(SIZEOF(.data) == 0 && SIZEOF(.bss) == 0, "Startup does not initialize writable globals")
