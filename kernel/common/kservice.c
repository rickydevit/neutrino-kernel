#include "kservice.h"
#include "kernel/common/device/serial.h"
#include "interrupts.h"
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <neutrino/lock.h>
#include <neutrino/macros.h>

// Private functions declarations

void kput(char* message, ...);
void kdbg(char* message, ...);
void klog(char* message, ...);
void kwarn(char* message, ...);
void kerr(char* message, ...);
void kpanic(Fatal fatal_error, ...);

static Lock l = NewLock;

// Public functions

//* Initialize the kernel services
void init_kservice() {
    ks._helper = serial_write_string;
    ks._put = kput;
    ks.log = klog;
    ks.dbg = kdbg;
    ks.warn = kwarn;
    ks.err = kerr;
    ks.fatal = kpanic;

    ks.log("Kernel services initialized.");
}

//* Set a new function for the given Kernel service
// @param type the type of service to change
// @param func the new function to be substituted in the kernel services
void set_kservice(enum KSERVICE_TYPE type, void (*func)) {
    switch (type) {
        case KSERVICE_HELPER: ks._helper = func; break;
        case KSERVICE_DEBUG: ks.dbg = func; break;
        case KSERVICE_LOG: ks.log = func; break;
        case KSERVICE_WARNING: ks.warn = func; break;
        case KSERVICE_ERROR: ks.err = func; break;
        case KSERVICE_FATAL: ks.fatal = func; break;
        default: break;
    }
}

// Private functions

void kput(char* message, ...) {
    va_list args; va_start(args, message);
    char buf[2048] = {0};
    ks._helper(vstrf(message, buf, args));
}

void volatile_fun klog(char* message, ...) {
    disable_interrupts();
    lock(&l);
    va_list args; va_start(args, message);
    char buf[2048] = {0};
    ks._helper("[LOG] ");
    ks._helper(vstrf(message, buf, args));
    ks._helper("\n");
    unlock(&l);
    enable_interrupts();
}

void volatile_fun kdbg(char* message, ...) {
    disable_interrupts();
    lock(&l);
    va_list args; va_start(args, message);
    char buf[2048] = {0};
    ks._helper("[DEBUG] ");
    ks._helper(vstrf(message, buf, args));
    ks._helper("\n");
    unlock(&l);
    enable_interrupts();
}

void volatile_fun kwarn(char* message, ...) {
    disable_interrupts();
    lock(&l);
    va_list args; va_start(args, message);
    char buf[2048] = {0};
    ks._helper("[WARN] ");
    ks._helper(vstrf(message, buf, args));
    ks._helper("\n");
    unlock(&l);
    enable_interrupts();
}

void volatile_fun kerr(char* message, ...) {
    disable_interrupts();
    lock(&l);
    va_list args; va_start(args, message);
    char buf[2048] = {0};
    ks._helper("[ERR] ");
    ks._helper(vstrf(message, buf, args));
    ks._helper("\n");
    unlock(&l);
    enable_interrupts();
}

void volatile_fun kpanic(Fatal fatal_error, ...) {
    disable_interrupts();
    lock(&l);
    va_list args; va_start(args, fatal_error);
    char buf[2048] = {0}, cbuf[32] = {0};
    ltoa((uint64_t)fatal_error.code, 16, cbuf);
    ks._helper("[FATAL] (0x");
    ks._helper(cbuf);
    ks._helper(") ");
    ks._helper(vstrf(fatal_error.message, buf, args));
    ks._helper("\n");

    asm volatile ("hlt");
}
