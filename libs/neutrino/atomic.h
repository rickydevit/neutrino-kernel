#pragma once
#include <stdint.h>
#include <stdbool.h>

void atomic_set_qword(volatile uintptr_t ptr, uint64_t value);
void atomic_set_dword(volatile uintptr_t ptr, uint32_t value);
void atomic_set_word(volatile uintptr_t ptr, uint16_t value);
void atomic_set_byte(volatile uintptr_t ptr, uint8_t value);

uint64_t atomic_get_qword(const volatile uintptr_t ptr);
uint32_t atomic_get_dword(const volatile uintptr_t ptr);
uint16_t atomic_get_word(const volatile uintptr_t ptr);
uint8_t atomic_get_byte(const volatile uintptr_t ptr);

bool atomic_test_and_set(volatile uint8_t* ptr);
void atomic_release(volatile uint8_t* ptr);
