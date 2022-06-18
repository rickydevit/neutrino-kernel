#include "atomic.h"
#include "macros.h"
#include <stdint.h>
#include <stdbool.h>

// --- ATOMIC SET -------------------------------

inline void unoptimized atomic_set_qword(volatile uintptr_t ptr, uint64_t value) {
    __atomic_store_n((volatile uint64_t*)ptr, value, __ATOMIC_SEQ_CST);
}

inline void unoptimized atomic_set_dword(volatile uintptr_t ptr, uint32_t value) {
    __atomic_store_n((volatile uint32_t*)ptr, value, __ATOMIC_SEQ_CST);
}

inline void unoptimized atomic_set_word(volatile uintptr_t ptr, uint16_t value) {
    __atomic_store_n((volatile uint16_t*)ptr, value, __ATOMIC_SEQ_CST);
}

inline void unoptimized atomic_set_byte(volatile uintptr_t ptr, uint8_t value) {
    __atomic_store_n((volatile uint8_t*)ptr, value, __ATOMIC_SEQ_CST);
}

// --- ATOMIC READ ------------------------------

inline uint64_t unoptimized atomic_get_qword(const volatile uintptr_t ptr) {
    return __atomic_load_n((const volatile uint64_t*)ptr, __ATOMIC_SEQ_CST);
}

inline uint32_t unoptimized atomic_get_dword(const volatile uintptr_t ptr) {
    return __atomic_load_n((const volatile uint32_t*)ptr, __ATOMIC_SEQ_CST);
}

inline uint16_t unoptimized atomic_get_word(const volatile uintptr_t ptr) {
    return __atomic_load_n((const volatile uint16_t*)ptr, __ATOMIC_SEQ_CST);
}

inline uint8_t unoptimized atomic_get_byte(const volatile uintptr_t ptr) {
    return __atomic_load_n((const volatile uint8_t*)ptr, __ATOMIC_SEQ_CST);
}

// --- ATOMIC LOCK ------------------------------

inline bool unoptimized atomic_test_and_set(volatile uint8_t* ptr) {
    return __atomic_test_and_set((volatile uint8_t*)ptr, __ATOMIC_SEQ_CST);
}

inline void unoptimized atomic_release(volatile uint8_t* ptr) {
    __atomic_clear((volatile uint8_t*)ptr, __ATOMIC_RELEASE);
}
