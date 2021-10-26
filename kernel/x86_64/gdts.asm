[BITS 64]

section .text

global load_gdt
load_gdt:
    cli
    lgdt [rdi]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    jmp ._load_gdt

._load_gdt:
    ret
