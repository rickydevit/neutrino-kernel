#pragma once

#define unoptimized     __attribute__((optimize("O0")))
#define packed          __attribute__((packed))
#define aligned(align)  __attribute__((aligned(align)))

#define Max(a,b)        ((a > b) ? a : b)
#define Min(a,b)        ((a < b) ? a : b)

#define AlignUp(addr, align)    ((addr + align-1) & ~(align-1))
