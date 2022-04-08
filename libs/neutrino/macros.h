#pragma once

#define unoptimized     __attribute__((optimize("O0")))
#define packed          __attribute__((packed))
#define aligned(align)  __attribute__((aligned(align)))
