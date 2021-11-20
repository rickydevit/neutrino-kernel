#include "string.h"
#include "integer.h"
#include "stdbool.h"
#include "stdarg.h"
#include "kernel/common/device/serial.h"

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

// *Convert an unsigned long to its string equivalent
// @param i the unsigned long to convert
// @param base the base to convert the unsigned long to
// @param buf the buffer to use during the conversion
// @return the converted string 
char* ltoa(uint64_t i, unsigned base, char* buf) {
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

// *Convert a signed long to its string equivalent
// @param i the signed long to convert
// @param base the base to convert the signed long to
// @param buf the buffer to use during the conversion
// @return the converted string 
void ltoa_s(int64_t i, unsigned base, char* buf) {
    if (base > 16) return;
    if (i < 0) {
        *buf++ = '-';
        i *= -1;
    }
    ltoa(i, base, buf);
}

// *Return a formatted string containing the specified arguments
// @param istr the string to format
// @param buffer the buffer to use during the conversion
// @param ... the arguments to be formatted in the string
// @return the formatted string
char* strf(const char* istr, char buffer[], ...) {
    if (!istr) return 0;

    va_list args;
    va_start(args, buffer);

    return vstrf(istr, buffer, args);
}

// *Return a formatted string containing the specified arguments
// @param istr the string to format
// @param buffer the buffer to use during the conversion
// @param args the arguments list to be formatted in the string
// @return the formatted string
char* vstrf(const char* istr, char buffer[], va_list args) {
    if (!istr) return 0;

    char *buffer_p = buffer;
    unsigned int i=0;

    while (istr[i] != '\0') {
        switch (istr[i]) {
            case '%':
                switch (istr[i+1]) {
                    case 'c': {
                        char* c = va_arg(args, char*);
                        for (int j=0; j < strlen(c); j++) {
                            *buffer_p = c[j];
                            buffer_p++;
                        }
                        i++;
                        break;
                    }

                    case 'u': {
                        uint64_t c = va_arg(args, uint64_t);
                        char str[32] = {0};
                        ltoa(c, 10, str);
                        for (int j=0; j < strlen(str); j++) {
                            *buffer_p = str[j];
                            buffer_p++;
                        }
                        i++;
                        break;
                    }

                    case 'd':
                    case 'i': {
                        int64_t c = va_arg(args, int64_t);
                        char str[32] = {0};
                        ltoa_s(c, 10, str);
                        for (int j=0; j < strlen(str); j++) {
                            *buffer_p = str[j];
                            buffer_p++;
                        }
                        i++;
                        break;
                    }

                    case 'X':
                    case 'x': {
                        uint64_t c = va_arg(args, uint64_t);
                        char str[32] = {0};
                        ltoa(c, 16, str);
                        *buffer_p++ = '0';
                        *buffer_p++ = 'x';
                        for (int j=0; j < strlen(str); j++) {
                            *buffer_p = str[j];
                            buffer_p++;
                        }
                        i++;
                        break;
                    }

                    case 'b': {
                        int64_t c = va_arg(args, int64_t);
                        char str[128] = {0};
                        ltoa(c, 2, str);
                        for (int j=0; j < strlen(str); j++) {
                            *buffer_p = str[j];
                            buffer_p++;
                        }
                        i++;
                        break;
                    }
                    default:
                        va_end(args);
                        return 1;
                }
                break;

            default:
                *buffer_p = istr[i];
                buffer_p++;
                break;
        }
        i++;
    }

    *buffer_p = '\0';
    va_end(args);
    return buffer;
}
