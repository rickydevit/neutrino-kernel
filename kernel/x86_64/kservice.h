#pragma once

struct KernelService {
    void (*dbg)     (char* message, ...);
    void (*log)     (char* message, ...);
    void (*err)     (char* message, ...);
    void (*panic)   (char* message, ...);
};

//? Global KernelService
struct KernelService ks;

void init_kservice();
