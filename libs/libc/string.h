#pragma once
#include "size_t.h"
#include "stdbool.h"
#include "stdarg.h"

unsigned int strlen(const char str[]);
bool strcmp(const char* s1, const char* s2);
char* strf(const char* str, char buffer[], ...);
char* vstrf(const char* istr, char buffer[], va_list args);
char* itoa(unsigned i, unsigned base, char* buf);
