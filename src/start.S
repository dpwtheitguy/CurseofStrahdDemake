.syntax unified
.cpu arm7tdmi
.arm

.section .gba_header, "ax", %progbits
.global _start
_start:
    b boot
    .space 156              @ Cartridge logo field, zero for emulator boot.
    .ascii "GBA HELLO   "  @ 12-byte title
    .ascii "AHLE"          @ Game code
    .ascii "00"            @ Maker code
    .byte 0x96, 0, 0       @ Fixed byte, unit code, device type
    .space 7
    .byte 0                @ Software version
    .byte 0x19             @ Header complement for bytes A0h through BCh
    .space 2

.section .text.boot, "ax", %progbits
.global boot
boot:
    msr cpsr_c, #0xdf       @ System mode with IRQ and FIQ disabled.
    ldr sp, =0x03007f00
    bl main
1:  b 1b
