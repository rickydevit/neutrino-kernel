#include "kservice.h"
#include "libs/libc/stdarg.h"
#include "libs/libc/string.h"
#include "kernel/common/device/serial.h"

// Private functions declarations

void kdbg(char* message, ...);
void klog(char* message, ...);
void kerr(char* message, ...);
void kpanic(char* message, ...);

// Public functions

//* Initialize the kernel services
void init_kservice() {
    ks._helper = serial_write_string;
    ks.log = klog;
    ks.dbg = kdbg;
    ks.err = kerr;
    ks.panic = kpanic;

    ks.dbg("Kernel services initialized.");
}

//* Set a new function for the given Kernel service
// @param type the type of service to change
// @param func the new function to be substituted in the kernel services
void set_kservice(enum KSERVICE_TYPE type, void (*func)) {
    switch (type) {
        case KSERVICE_HELPER: ks._helper = func; break;
        case KSERVICE_DEBUG: ks.dbg = func; break;
        case KSERVICE_LOG: ks.log = func; break;
        case KSERVICE_ERROR: ks.err = func; break;
        case KSERVICE_PANIC: ks.panic = func; break;
        default: break;
    }
}

// Private functions

void klog(char* message, ...) {
    va_list args; va_start(args, message);
    char buf[2048] = {0};
    ks._helper("[LOG] ");
    ks._helper(vstrf(message, buf, args));
    ks._helper("\n");
}

void kdbg(char* message, ...) {
    va_list args; va_start(args, message);
    char buf[2048] = {0};
    ks._helper("[DEBUG] ");
    ks._helper(vstrf(message, buf, args));
    ks._helper("\n");
}

void kerr(char* message, ...) {
    va_list args; va_start(args, message);
    char buf[2048] = {0};
    ks._helper("[ERR] ");
    ks._helper(vstrf(message, buf, args));
    ks._helper("\n");
}

void kpanic(char* message, ...) {
    va_list args; va_start(args, message);
    char buf[2048] = {0};
    ks._helper("[PANIC] ");
    ks._helper(vstrf(message, buf, args));
    ks._helper("\n");

    asm("hlt");
}
