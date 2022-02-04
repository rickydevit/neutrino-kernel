#pragma once

#define volatile_fun    __attribute__((optimize("O0")))
#define packed          __attribute__((packed))
#define aligned(align)  __attribute__((aligned(align)))
