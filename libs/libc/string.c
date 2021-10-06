#include "string.h"
#include "integer.h"
#include "stdbool.h"

char tbuf[32] = {0};
char bchars[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

// *Return the length of the given string [str]
// @param str the string to be measured
// @return the length of the string [str]
unsigned int strlen(const char str[]) {
    int l;
    for(l=0; str[l]!='\0'; l++);
    return l;
}

// *Comapre two strings [s1, s2] and return if they are equal
// @param s1 the first string to compare
// @param s2 the second string to compare
// @return true if the strings are equal, false otherwise
bool strcmp(const char* s1, const char* s2) {
    bool result = false;
    for (size_t i=0; ; i++) {
        if (s1[i] != s2[i]) return false;
        else result = true;
        if (s1[i] == '\0' || s2[i] == '\0') break;
    }

    return result;
}

// *Convert an unsigned integer to its string equivalent
// @param i the unsigned integer to convert
// @param base the base to convert the unsigned integer to
// @param buf the buffer to use during the conversion
// @return the converted string 
char* itoa(unsigned i, unsigned base, char* buf) {
    int pos = 0;
    int opos = 0;
    int top = 0;

    if (i == 0 || base > 16) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (i != 0) {
        tbuf[pos] = bchars[i % base];
        pos++;
        i /= base;
    }

    top=pos--;
    for (opos = 0; opos < top; pos--, opos++) {
        buf[opos] = tbuf[pos];
    }
    buf[opos] = 0;

    return buf;
}

// *Convert a signed integer to its string equivalent
// @param i the signed integer to convert
// @param base the base to convert the signed integer to
// @param buf the buffer to use during the conversion
// @return the converted string 
void itoa_s(int i, unsigned base, char* buf) {
    if (base > 16) return;
    if (i < 0) {
        *buf++ = '-';
        i *= -1;
    }
    itoa(i, base, buf);
}
