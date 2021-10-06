#include "integer.h"

// *Return the absolute value of the given [n] integer
// @param n the integer to be converted
// @return the absolute value of the given integer
int abs(int n) {
    return n < 0 ? -n : n;
}