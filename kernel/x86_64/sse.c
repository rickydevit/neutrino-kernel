#include "sse.h"
#include "libs/libc/stdbool.h"
#include "cpuid.h"
#include "kservice.h"

bool use_xsave = false;
uint64_t fpu_data[128] __attribute__((aligned(64)));

// === PRIVATE FUNCTIONS ========================

// *Check if SSE is available on the system
// @return true if SSE is available, false otherwise
bool has_sse() {
    return get_cpu_feature(CPUID_FEAT_EDX_SSE, false);
}

// *Check if XSAVE is available on the system
// @return true if XSAVE is available, false otherwise
bool has_xsave() {
    return get_cpu_feature(CPUID_FEAT_ECX_XSAVE, true);
}

// *Check if AVX is available on the system
// @return true if AVX is available, false otherwise
bool has_avx() {
    return get_cpu_feature(CPUID_FEAT_ECX_AVX, true);
}

// *Enable SSE in the system. Must first check if SSE is available with has_sse()
void __attribute__((optimize("O0"))) enable_sse() {
    uint64_t cr0, cr4;
    __asm__("mov %%cr0, %0" : "=g" (cr0) : );
    __asm__("mov %%cr4, %0" : "=g" (cr4) : );

    cr0 &= ~(1 << 2);       // clear CR0.EM bit
    cr0 |= (1 << 1);        // set CR0.MP bit
    cr4 |= (1 << 9);        // set CR4.OSFXSR bit
    cr4 |= (1 << 10);       // set CR4.OSXMMEXCPT bit

    __asm__("mov %0, %%cr0" : : "r" (cr0));
    __asm__("mov %0, %%cr4" : : "r" (cr4));
}

// *Enable XSAVE instruction in the system. Must first check if XSAVE is supported with has_xsave()
void __attribute__((optimize("O0"))) enable_xsave() {
    uint64_t cr4;
    __asm__("mov %%cr4, %0" : "=g" (cr4) : );   
    cr4 |= (1 << 18);
    __asm__("mov %0, %%cr4" : : "r" (cr4)); // enable XSAVE bit in CR4
    
    __asm__("xsetbv" : : "a" (XCR0_X87_STATE | XCR0_ENABLE_SSE), "c" (0), "d" (0));
}

// *Enable AVX instruction set in the system. Must first check if AVX is supported with has_avx()
void __attribute__((optimize("O0"))) enable_avx() {
    __asm__("xsetbv" : : "a" (XCR0_X87_STATE | XCR0_ENABLE_SSE | XCR0_ENABLE_AVX), "c" (0), "d" (0));
}

// === PUBLIC FUNCTIONS =========================

// *Initialize SSE in the system, if available
void init_sse() {
    if (!has_sse()) {
        ks.warn("SSE is not available in the system. Some feature may be limited");
        return;
    }

    enable_sse();
    ks.dbg("SSE has been enabled"); 

    if (has_xsave()) {
        enable_xsave();
        ks.dbg("XSAVE instruction set has been enabled"); 
        use_xsave = true;

        if (has_avx()) {
            enable_avx();
            ks.dbg("AVX instrucion set has been enabled");
        }
    }
    
    enable_fpu();    
    _avx_save((uintptr_t)fpu_data);
}

void save_sse_context(uint8_t* context) {
    if (use_xsave) _avx_save((uintptr_t)context);
    else _sse_save((uintptr_t)context);
}

void load_sse_context(uint8_t* context) {
    if (use_xsave) _avx_load((uintptr_t)context);
    else _sse_load((uintptr_t)context);
}
