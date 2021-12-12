#include "pic.h"
#include "kernel/common/device/port.h"
#include "kernel/common/kservice.h"
#include "libs/libc/stdint.h"

// === PRIVATE FUNCTIONS ========================

// *Small helper to send data to the desired pic port and wait
// @param port the pic port to send data to
// @param data the data to send
void inline send_pic(unsigned short port, uint8_t data) {
    port_byte_out(port, data);
    pic_wait();
}

// *Disable the 8259 PIC
void disable_pic() {
    port_byte_out(PIC1_DATA, 0xff);
    pic_wait();
    port_byte_out(PIC2_DATA, 0xff);
}

// === PUBLIC FUNCTIONS =========================

void init_pic() {
    ks.log("Initializing PIC...");
    send_pic(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  
    send_pic(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    send_pic(PIC1_DATA, PIC1_OFFSET);
    send_pic(PIC2_DATA, PIC2_OFFSET);

    send_pic(PIC1_DATA, 0x04);
    send_pic(PIC2_DATA, 0x02);

    send_pic(PIC1_DATA, 0x01);
    send_pic(PIC2_DATA, 0x01);

    ks.log("Disabling PIC for future APIC initilization");
    disable_pic();
    ks.log("PIC has been initialized.");
}
