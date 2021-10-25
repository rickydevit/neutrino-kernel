#include "kservice.h"
#include "libs/libc/stdarg.h"
#include "libs/libc/string.h"

// Private functions declarations

void kdbg(char* message, ...);
void klog(char* message, ...);
void kerr(char* message, ...);
void kpanic(char* message, ...);

// Public functions

//* Initialize the kernel services
void init_kservice() {
    ks.log = klog;
    ks.dbg = kdbg;
    ks.err = kerr;
    ks.panic = kpanic;

    ks.dbg("Kernel services initialized.");
}

// Private functions

void klog(char* message, ...) {
    va_list args; va_start(args, message);
    char buf[2048] = {0};
    serial_write_string("[LOG] ");
    serial_write_string(vstrf(message, buf, args));
    serial_write_string("\n");
}

void kdbg(char* message, ...) {
    va_list args; va_start(args, message);
    char buf[2048] = {0};
    serial_write_string("[DEBUG] ");
    serial_write_string(vstrf(message, buf, args));
    serial_write_string("\n");
}

void kerr(char* message, ...) {
    va_list args; va_start(args, message);
    char buf[2048] = {0};
    serial_write_string("[ERR] ");
    serial_write_string(vstrf(message, buf, args));
    serial_write_string("\n");
}

void kpanic(char* message, ...) {
    va_list args; va_start(args, message);
    char buf[2048] = {0};
    serial_write_string("[PANIC] ");
    serial_write_string(vstrf(message, buf, args));
    serial_write_string("\n");

    asm("hlt");
}
