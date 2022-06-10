#pragma once

#define unoptimized     __attribute__((optimize("O0")))
#define packed          __attribute__((packed))
#define aligned(align)  __attribute__((aligned(align)))
#define cleanup(func)  __attribute__((cleanup(func)))

#define Max(a,b)        ((a > b) ? a : b)
#define Min(a,b)        ((a < b) ? a : b)

#define _Concat(a,b)     a##b
#define Concat(a,b)      _Concat(a,b)

#define AlignUp(addr, align)    ((addr + align-1) & ~(align-1))
#define GetMemberOffset(T, member)  (size_t)(uintptr_t)(&((T*)0)->member)

#define GenerateEnum(list)  list, 
#define GenerateString(list) #list, 
