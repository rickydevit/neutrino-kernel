#pragma once
#include <stdint.h>

void init_syscall();
void syscall_set_gs(uintptr_t addr);
