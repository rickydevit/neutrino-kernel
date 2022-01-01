#include "pic.h"
#include "kernel/common/device/port.h"
#include "kernel/common/kservice.h"
#include "stdint.h"
#include "interrupts.h"

// === PRIVATE FUNCTIONS ========================

uint16_t pic_read_reg(uint32_t ocw3) {
    port_byte_out(PIC1_COMMAND, ocw3);
    port_byte_out(PIC2_COMMAND, ocw3);
    return (port_byte_in(PIC2_COMMAND) << 8 | port_byte_in(PIC1_COMMAND));
}

// *Small helper to send data to the desired pic port and wait
// @param port the pic port to send data to
// @param data the data to send
void inline pic_send(unsigned short port, uint8_t data) {
    port_byte_out(port, data);
    pic_wait();
}

// *Disable the 8259 PIC
void pic_disable() {
    pic_send(PIC1_DATA, 0xff);
    pic_send(PIC2_DATA, 0xff);
}

// === PUBLIC FUNCTIONS =========================

void init_pic() {
    ks.log("Initializing PIC...");
    disable_interrupts();

    pic_send(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  
    pic_send(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    pic_send(PIC1_DATA, PIC1_OFFSET);
    pic_send(PIC2_DATA, PIC2_OFFSET);

    pic_send(PIC1_DATA, 0x04);
    pic_send(PIC2_DATA, 0x02);

    pic_send(PIC1_DATA, 0x01);
    pic_send(PIC2_DATA, 0x01);

    pic_disable();

    ks.log("PIC has been initialized.");
}

// *Send a EOI signal to both PICs
void pic_eoi() {
    port_byte_out(PIC1_COMMAND, PIC_EOI);
    port_byte_out(PIC2_COMMAND, PIC_EOI);
}

// *Get the IRR value from the PIC
// @return the IRR value
uint16_t pic_get_irr() {
    return pic_read_reg(PIC_READ_IRR);
}

// *Get the ISR value from the PIC
// @return the ISR value
uint16_t pic_get_isr() {
    return pic_read_reg(PIC_READ_ISR);
}
