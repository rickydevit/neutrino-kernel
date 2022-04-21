#include "interrupts.h"
#include "kservice.h"
#include "pic.h"
#include "device/apic.h"
#include "device/time/pit.h"
#include "kernel/common/device/port.h"
#include "kernel/common/tasks/context.h"
#include "kernel/common/memory/space.h"
#include "kernel/common/tasks/scheduler.h"
#include "kernel/common/cpu.h"
#include <stdint.h>
#include <neutrino/macros.h>
#include <neutrino/lock.h>
#include <neutrino/atomic.h>

struct IDT_entry IDT[IDT_SIZE];

const char *interrupt_exception_name[] = {
    "Division By 0",
    "Debug interrupt",
    "NMI (Non Maskable Interrupt)",
    "Breakpoint interrupt",
    "invalid", 
    "table overflow",
    "Invalid opcode",
    "No FPU",
    "Double fault",
    "invalid", 
    "invalid TSS",
    "Segment not present",
    "invalid stack",
    "General protection fault",
    "Page fault",
    "invalid",
    "x87 FPU fault",
    "Alignment fault",
    "Machine check fault",
    "SIMD floating point exception",
    "vitualisation excpetion",
    "control protection exception",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "invalid",
    "invalid"
	};

// === PRIVATE FUNCTIONS ========================

// *Add a new entry to the IDT table
// @param irq the Interrupt number
// @param isr the pointer to the Interrupt Service Routine
// @param ist the offset of the interrupt stack table stored in the TSS
// @param type_attr the attributes for the newly added idt entry
void unoptimized set_idt_entry(uint32_t irq, int(*isr)(), uint16_t ist, uint8_t type_attr) {
	uint64_t irq_address = (uint64_t)isr;
	IDT[irq].offset_lowerbits = (irq_address) & 0xffff;
	IDT[irq].selector = KERNEL_CODE;
	IDT[irq].zero = 0;
	IDT[irq].ist = ist;
	IDT[irq].type_attr = type_attr;
	IDT[irq].offset_middlebits = ((irq_address) >> 16) & 0xffff;
	IDT[irq].offset_higherbits = ((irq_address) >> 32) & 0xffffffff;
    IDT[irq].zero2 = 0;
}

// *Log the interrupt stack passed to the function. Useful for debugging interrupts stack frames
// @param stack the pointer to the interrupt stack to be logged
void unoptimized log_interrupt(InterruptStack* stack) {
    ks.dbg(" ========== INTERRUPT FRAME LOG ==========\n\
            Got interrupt %u with error_code %x on cpu #%d \n\
            Previous stack frame was %x \n\
            rax: %x | rbx: %x | rcx: %x | rdx: %x \n\
            rsi: %x | rdi: %x | rbp: %x \n\
            r8: %x | r9: %x | r10: %x \n\
            r11: %x | r12: %x | r13: %x \n\
            r14: %x | r15: %x \n\
            rflags: %x | rsp: %x \n\
            cs: %x | ss: %x | rip: %x\n\
         ====== END OF INTERRUPT FRAME LOG =======", 
            stack->irq, stack->error_code, get_current_cpu()->id, 
            stack->rsp,
            stack->rax, stack->rbx, stack->rcx, stack->rdx, 
            stack->rsi, stack->rdi, stack->rbp,
            stack->r8, stack->r9, stack->r10, 
            stack->r11, stack->r12, stack->r13,
            stack->r14, stack->r15, 
            stack->rflags, stack->rsp, 
            stack->cs, stack->ss, stack->rip);
}

// === PUBLIC FUNCTIONS =========================

// *Initialize the Interrupt Descriptor Table
void init_idt() {
	for (uint64_t i = 0; i < 48; i++) {
		if (i == 0x0e || i == 0x08 || i == 0x0d || i == 0x20) 
			set_idt_entry(i, _interrupt_vector[i], 1, INTERRUPT_GATE);
		else
			set_idt_entry(i, _interrupt_vector[i], 0, INTERRUPT_GATE);
	}

    /* fill the IDT descriptor */
	struct IDT_pointer idt_ptr; 
	idt_ptr.size = sizeof(struct IDT_entry) * IDT_SIZE;
	idt_ptr.offset = (uint64_t)&IDT[0];

    ks.dbg("IDT built at %x, size is %x. Loading to register...", idt_ptr.offset, idt_ptr.size);
	load_idt((uintptr_t)&idt_ptr);
	ks.log("Interrupt Descriptor Table loaded successfully.");
}

InterruptStack* exception_handler(InterruptStack* stack) {
    switch (stack->irq) {
        case 8:     // DF
        case 10:    // TS
        case 11:    // NP
        case 12:    // SS
        case 13:    // GP
            log_interrupt(stack);
            ks.err("General Protection Fault detected.");
            break;
        case 17:    // AC
        case 18:    // MC
        case 21:    // CP
        case 29:    // VC
        case 30:    // SX
            log_interrupt(stack);
            ks.fatal(FatalError(INTERRUPT_EXCEPTION, "Neutrino encountered a fatal exception!"));
            break;
        case 14:    // PF
            log_interrupt(stack);
            pagefault_handler(stack);
            break;

        default: 
           log_interrupt(stack);
           break; 
    }
    
    apic_eoi();
    return stack;
}

InterruptStack* unoptimized interrupt_handler(InterruptStack* stack) {

    if (stack->irq == APIC_TIMER_IRQ) {     // timer interrupt, do task switch
        if (scheduler.ready) {
            volatile Cpu* cpu = get_current_cpu();
            if (atomic_get_byte((uintptr_t)&(cpu->tasks.is_switching.flag)) == UNLOCKED) {
                lock((Lock*)&(cpu->tasks.is_switching));
                
                if (cpu->tasks.current != nullptr && !IsTaskNeverRun(cpu->tasks.current))
                    context_save(cpu->tasks.current->context, stack);     // save context to task
                sched_cycle(cpu);
                context_load(cpu->tasks.current->context, stack);     // load context from task

                space_switch(cpu->tasks.current->space);              // switch space
                
                unlock((Lock*)&(cpu->tasks.is_switching));
            }
        }
    }

    apic_eoi();
    return stack;
}

void pagefault_handler(InterruptStack* stack) {
	uint64_t faulting_address;
   	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

	int present = stack->error_code & 0x1;
	int rw = stack->error_code & 0x2;           // Write operation?
    int us = stack->error_code & 0x4;           // Processor was in user-mode?
    int reserved = stack->error_code & 0x8;     // Overwritten CPU-reserved bits of page entry?

	ks.err("Page fault at address %x. %c, %c, %c, Reserved bits %c", faulting_address, (present == 1) ? "Present" : "Non-present", 
           (rw == 1) ? "Write" : "Read", (us == 1) ? "User" : "Supervisor", (reserved == 1) ? "set" : "untoched"); 

    disable_interrupts();
    asm("hlt");
}