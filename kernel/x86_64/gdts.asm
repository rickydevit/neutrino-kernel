[BITS 64]
section .text

global load_gdt
load_gdt:
    cli
    lgdt [rdi]

    ; load data registers
    mov eax, 0x20
    mov ds, eax
    mov es, eax
    mov ss, eax

    push qword 0x18             ; push KERNEL_DATA selector
    push qword ret_from_gdt     ; push RETURN_TRAMPOLINE address
    o64 retf                    ; perform a far return to 0x18:ret_from_gdt by picking them from the stack

ret_from_gdt:
    ret