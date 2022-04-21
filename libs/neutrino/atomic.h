#pragma once
#include <stdint.h>
#include <stdbool.h>

void atomic_set_qword(uintptr_t ptr, uint64_t value);
void atomic_set_dword(uintptr_t ptr, uint32_t value);
void atomic_set_word(uintptr_t ptr, uint16_t value);
void atomic_set_byte(uintptr_t ptr, uint8_t value);

uint64_t atomic_get_qword(uintptr_t ptr);
uint32_t atomic_get_dword(uintptr_t ptr);
uint16_t atomic_get_word(uintptr_t ptr);
uint8_t atomic_get_byte(uintptr_t ptr);

bool atomic_test_and_set(uint8_t* ptr);
void atomic_release(uint8_t* ptr);
