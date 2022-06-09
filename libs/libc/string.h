#pragma once
#include "size_t.h"
#include "stdbool.h"
#include "stdarg.h"

unsigned int strlen(const char str[]);
bool strcmp(const char* s1, const char* s2);
int32_t strncmp(const char* s1, const char* s2, size_t len);
bool strcpy(const char *src, char *dest);
char* strf(const char* str, char buffer[], ...);
char* vstrf(const char* istr, char buffer[], va_list args);

char* itoa(unsigned i, unsigned base, char* buf);
void itoa_s(int i, unsigned base, char* buf);
char* ltoa(uint64_t i, unsigned base, char* buf);
void ltoa_s(int64_t i, unsigned base, char* buf);

void memcpy(uint8_t *source, uint8_t *dest, int nbytes);
void memset(uint8_t *dest, uint8_t val, uint32_t len);