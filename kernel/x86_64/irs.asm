[BITS 64]

section .text

global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

global exc0
global exc1
global exc2
global exc3
global exc4
global exc5
global exc6
global exc8
global exc14
 
global load_idt
 
extern irq0_handler
extern irq1_handler
extern irq2_handler
extern irq3_handler
extern irq4_handler
extern irq5_handler
extern irq6_handler
extern irq7_handler
extern irq8_handler
extern irq9_handler
extern irq10_handler
extern irq11_handler
extern irq12_handler
extern irq13_handler
extern irq14_handler
extern irq15_handler

extern exc0_handler
extern exc1_handler
extern exc2_handler
extern exc3_handler
extern exc4_handler
extern exc5_handler
extern exc6_handler
extern exc8_handler
extern exc14_handler

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

%macro _EXCEPTION_COMMON 1
exc%1:
  cld
  pushall

  call exc%1_handler

  popall
  sti
  iretq
%endmacro

%macro _INTERRUPT_COMMON 1
irq%1:
  cld
  pushall

  call irq%1_handler

  popall
  sti
  iretq
%endmacro

_EXCEPTION_COMMON 0   ; DIVIDE EXCEPTION
_EXCEPTION_COMMON 1   ; DEBUG EXCEPTION
_EXCEPTION_COMMON 2   ; INTEL RESERVED
_EXCEPTION_COMMON 3   ; BREAKPOINT
_EXCEPTION_COMMON 4   ; OVERFLOW
_EXCEPTION_COMMON 5   ; BOUNDS CHECK 
_EXCEPTION_COMMON 6   ; INVALID OPCODE
_EXCEPTION_COMMON 8   ; DOUBLE FAULT
exc14:                ; PAGE FAULT
  cld
  pushall
  mov rdi, rsp

  call exc14_handler

  mov rsp, rax
  popall
  sti
  iretq

; IRQs

_INTERRUPT_COMMON 0 
_INTERRUPT_COMMON 1
_INTERRUPT_COMMON 2
_INTERRUPT_COMMON 3
_INTERRUPT_COMMON 5
_INTERRUPT_COMMON 6
_INTERRUPT_COMMON 7
_INTERRUPT_COMMON 8
_INTERRUPT_COMMON 9
_INTERRUPT_COMMON 10
_INTERRUPT_COMMON 11
_INTERRUPT_COMMON 12
_INTERRUPT_COMMON 13 
_INTERRUPT_COMMON 14
_INTERRUPT_COMMON 15

load_idt:
	lidt [edi]
	sti
	ret
