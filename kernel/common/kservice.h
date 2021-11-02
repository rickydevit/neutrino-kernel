#pragma once

struct KernelService {
    void (*dbg)     (char* message, ...);
    void (*log)     (char* message, ...);
    void (*err)     (char* message, ...);
    void (*panic)   (char* message, ...);

    void (*_put) (char* message, ...);
    void (*_helper) (char* message);
};

enum KSERVICE_TYPE {
    KSERVICE_HELPER,
    KSERVICE_LOG,
    KSERVICE_DEBUG,
    KSERVICE_ERROR,
    KSERVICE_PANIC
};

//? Global KernelService
struct KernelService ks;

void init_kservice();
void set_kservice(enum KSERVICE_TYPE, void (*));
