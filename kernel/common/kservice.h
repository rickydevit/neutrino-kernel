#pragma once

enum KERNEL_FATAL {
    //! reserved                    | 0x000 - 0x0ff
    INTERNAL_TEST       = 0x000,
    NOT_IMPLEMENTED     = 0x001,

    //! core hardware not found     | 0x100 - 0x1ff
    NO_PIT              = 0x100,
    NO_FRAMEBUFFER      = 0x101,
    NO_LAPIC            = 0x102,

    //! fatal exceptions            | 0x200 - 0x2ff
    GENERIC_EXCEPTION   = 0x200,
    INTERRUPT_EXCEPTION = 0x201,
    OUT_OF_MEMORY       = 0x202,
};

typedef struct __fatal_t {
    enum KERNEL_FATAL code;
    char* message;
} fatal_t;

#define FatalError(code, message) (fatal_t){code, message}

struct KernelService {
    void (*dbg)     (char* message, ...);
    void (*log)     (char* message, ...);
    void (*warn)    (char* message, ...);
    void (*err)     (char* message, ...);
    void (*fatal)   (fatal_t fatal_error, ...);

    void (*_put) (char* message, ...);
    void (*_helper) (char* message);
};

enum KSERVICE_TYPE {
    KSERVICE_HELPER,
    KSERVICE_LOG,
    KSERVICE_DEBUG,
    KSERVICE_WARNING,
    KSERVICE_ERROR,
    KSERVICE_FATAL
};

//? Global KernelService
struct KernelService ks;

void init_kservice();
void set_kservice(enum KSERVICE_TYPE, void (*));
