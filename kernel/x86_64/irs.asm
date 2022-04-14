[BITS 64]

section .text
 
global load_idt
 
extern interrupt_handler
extern exception_handler

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
    pop rax
%endmacro

%macro _INT_NAME 1
dq irq%1
%endmacro

%macro _EXCEPTION_COMMON 1
irq%1:
  push qword 0xff     ; push dummy error
  push qword %1       ; push interrupt number
  jmp _generic_exception
%endmacro

%macro _EXCEPTION_WERROR 1
irq%1:
  ; cpu will push the error code
  push qword %1       ; push interrupt number
  jmp _generic_exception
%endmacro

%macro _INTERRUPT_COMMON 1
irq%1:
  push qword 0xff     ; push dummy error code
  push qword %1       ; push interrupt number
  jmp _generic_interrupt
%endmacro

_generic_exception:
  cld
  pushall
  
  mov rdi, rsp
  call exception_handler
  mov rsp, rax

  popall
  add rsp, 16         ; pop interrupt number
  iretq

_generic_interrupt:
  cld
  pushall

  mov rdi, rsp
  call interrupt_handler
  mov rsp, rax

  popall
  add rsp, 16         ; pop interrupt number
  iretq

_EXCEPTION_COMMON 0   ; DIVIDE EXCEPTION
_EXCEPTION_COMMON 1   ; DEBUG EXCEPTION
_EXCEPTION_COMMON 2   ; INTEL RESERVED
_EXCEPTION_COMMON 3   ; BREAKPOINT
_EXCEPTION_COMMON 4   ; invalid in 64 bit
_EXCEPTION_COMMON 5   ; BOUNDS CHECK 
_EXCEPTION_COMMON 6   ; INVALID OPCODE
_EXCEPTION_COMMON 7   ; NO FPU
_EXCEPTION_WERROR 8   ; DOUBLE FAULT
_EXCEPTION_COMMON 9   ; not used
_EXCEPTION_WERROR 10  ; INVALID TSS
_EXCEPTION_WERROR 11  ; SEGMENT NOT PRESENT 
_EXCEPTION_WERROR 12  ; INVALID STACK
_EXCEPTION_WERROR 13  ; GENERAL PROTECTION FAULT
_EXCEPTION_WERROR 14  ; PAGE FAULT
_EXCEPTION_COMMON 15  ; invalid
_EXCEPTION_COMMON 16  ; x87 FPU FAULT
_EXCEPTION_WERROR 17  ; ALIGNMENT FAULT
_EXCEPTION_COMMON 18
_EXCEPTION_COMMON 19
_EXCEPTION_COMMON 20
_EXCEPTION_WERROR 21
_EXCEPTION_COMMON 22
_EXCEPTION_COMMON 23
_EXCEPTION_COMMON 24
_EXCEPTION_COMMON 25
_EXCEPTION_COMMON 26
_EXCEPTION_COMMON 27
_EXCEPTION_COMMON 28
_EXCEPTION_WERROR 29
_EXCEPTION_WERROR 30
_EXCEPTION_COMMON 31

_INTERRUPT_COMMON 32 
_INTERRUPT_COMMON 33
_INTERRUPT_COMMON 34
_INTERRUPT_COMMON 35
_INTERRUPT_COMMON 36
_INTERRUPT_COMMON 37
_INTERRUPT_COMMON 38
_INTERRUPT_COMMON 39
_INTERRUPT_COMMON 40
_INTERRUPT_COMMON 41
_INTERRUPT_COMMON 42
_INTERRUPT_COMMON 43
_INTERRUPT_COMMON 44 
_INTERRUPT_COMMON 45
_INTERRUPT_COMMON 46
_INTERRUPT_COMMON 47

load_idt:
	lidt [rdi]
	ret

section .data
global _interrupt_vector

_interrupt_vector:
  _INT_NAME 0
  _INT_NAME 1
  _INT_NAME 2
  _INT_NAME 3
  _INT_NAME 4
  _INT_NAME 5
  _INT_NAME 6
  _INT_NAME 7
  _INT_NAME 8
  _INT_NAME 9
  _INT_NAME 10
  _INT_NAME 11
  _INT_NAME 12
  _INT_NAME 13
  _INT_NAME 14
  _INT_NAME 15
  _INT_NAME 16
  _INT_NAME 17
  _INT_NAME 18
  _INT_NAME 19
  _INT_NAME 20
  _INT_NAME 21
  _INT_NAME 22
  _INT_NAME 23
  _INT_NAME 24
  _INT_NAME 25
  _INT_NAME 26
  _INT_NAME 27
  _INT_NAME 28
  _INT_NAME 29
  _INT_NAME 30
  _INT_NAME 31

  _INT_NAME 32
  _INT_NAME 33
  _INT_NAME 34
  _INT_NAME 35
  _INT_NAME 36
  _INT_NAME 37
  _INT_NAME 38
  _INT_NAME 39
  _INT_NAME 40
  _INT_NAME 41
  _INT_NAME 42
  _INT_NAME 43
  _INT_NAME 44
  _INT_NAME 45
  _INT_NAME 46
  _INT_NAME 47
  