[BITS 64]

section .text

extern syscall_handler

%macro pushall 0
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro popall 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
%endmacro

ALIGN 4096
global _syscall
_syscall:
    swapgs
    mov [gs:0x8], rsp       ; set Context->syscall_ustack to rsp from syscall
    ; mov rsp, [gs:0x0]       ; set rsp to Context->syscall_kstack from task Context

    sti
    
    push qword 0x1b
    push qword [gs:0x8]
    push r11
    push qword 0x28
    push rcx

    push qword 0x0
    push qword 0x0

    cld
    pushall

    mov rdi, rsp
    mov rbp, 0

    call syscall_handler

    popall  ; except rax which contains the return value

    cli
    mov rsp, [gs:0x8]
    swapgs
    o64 sysret
