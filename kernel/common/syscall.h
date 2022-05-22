#pragma once
#include <neutrino/syscall.h>
#include <stdint.h>

SyscallResult syscall_execute(NeutrinoSyscall syscall_id, uintptr_t* args);
