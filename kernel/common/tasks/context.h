#pragma once

#include <stdint.h>

#ifdef __x86_64
#include "kernel/x86_64/tasks/context.h"
#else
#error "Unsupported platform"
#endif

typedef uint64_t ContextFlags;

#define IsUserTask(flags) ((flags) & (1<<0))

typedef struct __context Context;

Context* NewContext();
void DestroyContext(Context* context);
void context_init(Context* context, uintptr_t ip, uintptr_t sp, uintptr_t ksp, ContextFlags cflags);
