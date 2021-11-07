#include "idt.h"
#include "kservice.h"
#include "kernel/common/device/port.h"

struct IDT_entry IDT[IDT_SIZE];

// *Add a new entry to the IDT table
// @param irq the Interrupt number
// @param isr the pointer to the Interrupt Service Routine
// @param isr_address the address of the Interrupt Service Routine
// @param selector the segment selector for the newly added idt entry
// @param type_attr the attributes for the newly added idt entry
void add_entry(unsigned int irq, int(*isr)(), unsigned long *isr_address, unsigned short selector, unsigned char type_attr) {
	unsigned long irq_address = (unsigned long)isr;
	IDT[irq].offset_lowerbits = (irq_address) & 0xffff;
	IDT[irq].selector = selector;
	IDT[irq].zero = 0;
	IDT[irq].type_attr = type_attr;
	IDT[irq].offset_higherbits = ((irq_address) & 0xffff0000) >> 16;
    IDT[irq].zero2 = 0;
}

// *Initialize the Interrupt Descriptor Table
void init_idt() {
    unsigned long irq0_address;
    unsigned long irq1_address;
    unsigned long irq2_address;
    unsigned long irq3_address;          
    unsigned long irq5_address;
    unsigned long irq6_address;
    unsigned long irq7_address;
    unsigned long irq8_address;
    unsigned long irq9_address;          
    unsigned long irq10_address;
    unsigned long irq11_address;
    unsigned long irq12_address;
    unsigned long irq13_address;
    unsigned long irq14_address;          
    unsigned long irq15_address;      

	unsigned long exc0_address;
	unsigned long exc1_address;
	unsigned long exc2_address;
	unsigned long exc3_address;
	unsigned long exc4_address;
	unsigned long exc5_address;
	unsigned long exc6_address;
	unsigned long exc8_address;
	unsigned long exc14_address; //PAGE FAULT

	struct IDT_pointer idt_ptr; 
	uint64_t idt_address;

    add_entry(0, exc0, &exc0_address, 0x28, INTERRUPT_GATE);
	add_entry(1, exc1, &exc1_address, 0x28, INTERRUPT_GATE);
	add_entry(2, exc2, &exc2_address, 0x28, INTERRUPT_GATE);
	add_entry(3, exc3, &exc3_address, 0x28, INTERRUPT_GATE);
	add_entry(4, exc4, &exc4_address, 0x28, INTERRUPT_GATE);
	add_entry(5, exc5, &exc5_address, 0x28, INTERRUPT_GATE);
	add_entry(6, exc6, &exc6_address, 0x28, INTERRUPT_GATE);
	add_entry(8, exc8, &exc8_address, 0x28, INTERRUPT_GATE);
	add_entry(14, exc14, &exc14_address, 0x28, INTERRUPT_GATE);

	add_entry(32, irq0, &irq0_address, 0x28, INTERRUPT_GATE);
	add_entry(33, irq1, &irq1_address, 0x28, INTERRUPT_GATE);
	add_entry(34, irq2, &irq2_address, 0x28, INTERRUPT_GATE);
	add_entry(35, irq3, &irq3_address, 0x28, INTERRUPT_GATE);
	add_entry(37, irq5, &irq5_address, 0x28, INTERRUPT_GATE);
	add_entry(38, irq6, &irq6_address, 0x28, INTERRUPT_GATE);
	add_entry(39, irq7, &irq7_address, 0x28, INTERRUPT_GATE);
	add_entry(40, irq8, &irq8_address, 0x28, INTERRUPT_GATE);
	add_entry(41, irq9, &irq9_address, 0x28, INTERRUPT_GATE);
	add_entry(42, irq10, &irq10_address, 0x28, INTERRUPT_GATE);
	add_entry(43, irq11, &irq11_address, 0x28, INTERRUPT_GATE);
	add_entry(44, irq12, &irq12_address, 0x28, INTERRUPT_GATE);
	add_entry(45, irq13, &irq13_address, 0x28, INTERRUPT_GATE);
	add_entry(46, irq14, &irq14_address, 0x28, INTERRUPT_GATE);
	add_entry(47, irq15, &irq15_address, 0x28, INTERRUPT_GATE);

    /* fill the IDT descriptor */
    idt_address = (uint64_t)IDT;
	idt_ptr.size = sizeof(struct IDT_entry) * IDT_SIZE;
	idt_ptr.offset = idt_address;

    ks.dbg("IDT built at %x. Loading to register...", idt_address);
	load_idt(idt_ptr);
	ks.log("Interrupt Descriptor Table loaded successfully.");

	asm("sti");
}

void irq0_handler(void) {
	// increment_pit_ticks();
    portByteOut(0x20, 0x20); //EOI
}
 
void irq1_handler() {
	// keyboard_handler();
	portByteOut(0x20, 0x20); //EOI
}
 
void irq2_handler(void) {
    portByteOut(0x20, 0x20); //EOI
}
 
void irq3_handler(void) {
    portByteOut(0x20, 0x20); //EOI
}
 
void irq4_handler(void) {
    portByteOut(0x20, 0x20); //EOI
}
 
void irq5_handler(void) {
    portByteOut(0x20, 0x20); //EOI
}
 
void irq6_handler(void) {
    portByteOut(0x20, 0x20); //EOI
}
 
void irq7_handler(void) {
    portByteOut(0x20, 0x20); //EOI
}
 
void irq8_handler(void) {
    portByteOut(0xA0, 0x20);
    portByteOut(0x20, 0x20); //EOI          
}
 
void irq9_handler(void) {
    portByteOut(0xA0, 0x20);
    portByteOut(0x20, 0x20); //EOI
}
 
void irq10_handler(void) {
    portByteOut(0xA0, 0x20);
    portByteOut(0x20, 0x20); //EOI
}
 
void irq11_handler(void) {
    portByteOut(0xA0, 0x20);
    portByteOut(0x20, 0x20); //EOI
}
 
void irq12_handler(void) {
    portByteOut(0xA0, 0x20);
    portByteOut(0x20, 0x20); //EOI
}
 
void irq13_handler(void) {
    portByteOut(0xA0, 0x20);
    portByteOut(0x20, 0x20); //EOI
}
 
void irq14_handler(void) {
    portByteOut(0xA0, 0x20);
    portByteOut(0x20, 0x20); //EOI
}

void irq15_handler(void) {
    portByteOut(0xA0, 0x20);
    portByteOut(0x20, 0x20); //EOI
}

void exc0_handler() {
	ks.err("Divide error.");
	asm("hlt");
}

void exc1_handler() {
	ks.err("Debug exception. Halting system...");
	asm("hlt");
}

void exc2_handler() {
	ks.err("Intel reserved error. Halting system...");
	asm("hlt");
}

void exc3_handler() {
	ks.err("Breakpoint. Halting system...");
	asm("hlt");
}

void exc4_handler() {
	ks.err("Overflow exception. Halting system...");
	asm("hlt");
}

void exc5_handler() {
	ks.err("Bounds check error. Halting system...");
	asm("hlt");
}

void exc6_handler() {
	ks.err("Invalid OP code. Halting system...");
	asm("hlt");
}

void exc8_handler() {
	ks.err("Double fault. Halting system...");
	asm("hlt");
}

void exc14_handler(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n) {
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