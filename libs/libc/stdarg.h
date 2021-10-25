#pragma once

// #define STACKITEM int64_t
// #define VA_SIZE(TYPE) ((sizeof(TYPE) + sizeof(STACKITEM) - 1) & ~(sizeof(STACKITEM) - 1))
typedef __builtin_va_list va_list;

#define va_start(AP, LASTARG) __builtin_va_start(AP, LASTARG)
#define va_end(AP) __builtin_va_end(AP)
#define va_arg(AP, TYPE) __builtin_va_arg(AP, TYPE)
