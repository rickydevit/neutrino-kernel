#include "idt.h"
#include "kservice.h"
#include "kernel/common/device/port.h"
#include "libs/libc/stdint.h"

struct IDT_entry IDT[IDT_SIZE];

// *Add a new entry to the IDT table
// @param irq the Interrupt number
// @param isr the pointer to the Interrupt Service Routine
// @param ist the offset of the interrupt stack table stored in the TSS
// @param type_attr the attributes for the newly added idt entry
void set_idt_entry(uint32_t irq, int(*isr)(), uint16_t ist, uint8_t type_attr) {
	unsigned long irq_address = (unsigned long)isr;
	IDT[irq].offset_lowerbits = (irq_address) & 0xffff;
	IDT[irq].selector = KERNEL_CODE;
	IDT[irq].zero = 0;
	IDT[irq].ist = ist;
	IDT[irq].type_attr = type_attr;
	IDT[irq].offset_higherbits = ((irq_address) & 0xffff0000) >> 16;
    IDT[irq].zero2 = 0;
}

// *Initialize the Interrupt Descriptor Table
void init_idt() {

	for (uint64_t i = 0; i < 48; i++) {
		if (i == 14 || i == 32) 
			set_idt_entry(i, _interrupt_vector[i], 1, INTERRUPT_GATE);
		else
			set_idt_entry(i, _interrupt_vector[i], 0, INTERRUPT_GATE);
	}

    /* fill the IDT descriptor */
	struct IDT_pointer idt_ptr; 
	idt_ptr.size = sizeof(struct IDT_entry) * IDT_SIZE;
	idt_ptr.offset = (uint64_t)&IDT[0];

    ks.dbg("IDT built at %x, size is %x. Loading to register...", idt_ptr.offset, idt_ptr.size);
	load_idt((uint32_t)&idt_ptr);
	ks.log("Interrupt Descriptor Table loaded successfully.");
}

void exception_handler(uint64_t irq) {
	ks.log("got exception interrupt %u", irq);
}

void interrupt_handler(uint64_t irq) {
	ks.log("got interrupt %u", irq);
}

void pagefault_handler(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n) {
	unsigned long faulting_address, temp;
   	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

	int present = !(m & 0x1);
	int rw = m & 0x2;           // Write operation?
    int us = m & 0x4;           // Processor was in user-mode?
    int reserved = m & 0x8;     // Overwritten CPU-reserved bits of page entry?
    int id = m & 0x10; 

	ks.err("Page fault at address %x", faulting_address); 
	asm("hlt");
}