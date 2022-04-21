#include "atomic.h"
#include "macros.h"
#include <stdint.h>
#include <stdbool.h>

// --- ATOMIC SET -------------------------------

void unoptimized atomic_set_qword(uintptr_t ptr, uint64_t value) {
    __atomic_store_n((uint64_t*)ptr, value, __ATOMIC_SEQ_CST);
}

void unoptimized atomic_set_dword(uintptr_t ptr, uint32_t value) {
    __atomic_store_n((uint32_t*)ptr, value, __ATOMIC_SEQ_CST);
}

void unoptimized atomic_set_word(uintptr_t ptr, uint16_t value) {
    __atomic_store_n((uint16_t*)ptr, value, __ATOMIC_SEQ_CST);
}

void unoptimized atomic_set_byte(uintptr_t ptr, uint8_t value) {
    __atomic_store_n((uint8_t*)ptr, value, __ATOMIC_SEQ_CST);
}

// --- ATOMIC READ ------------------------------

uint64_t unoptimized atomic_get_qword(uintptr_t ptr) {
    return __atomic_load_n((uint64_t*)ptr, __ATOMIC_SEQ_CST);
}

uint32_t unoptimized atomic_get_dword(uintptr_t ptr) {
    return __atomic_load_n((uint32_t*)ptr, __ATOMIC_SEQ_CST);
}

uint16_t unoptimized atomic_get_word(uintptr_t ptr) {
    return __atomic_load_n((uint16_t*)ptr, __ATOMIC_SEQ_CST);
}

uint8_t unoptimized atomic_get_byte(uintptr_t ptr) {
    return __atomic_load_n((uint8_t*)ptr, __ATOMIC_SEQ_CST);
}

// --- ATOMIC LOCK ------------------------------

bool unoptimized atomic_test_and_set(uint8_t* ptr) {
    return __atomic_test_and_set(ptr, __ATOMIC_ACQUIRE);
}

void unoptimized atomic_release(uint8_t* ptr) {
    __atomic_clear(ptr, __ATOMIC_RELEASE);
}
