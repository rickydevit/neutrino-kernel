#pragma once
#include "stdint.h"

typedef struct __cmosReading {
    uint32_t second;
    uint32_t minute;
    uint32_t hour;
    uint8_t weekday;
    uint8_t monthday;
    uint8_t month;
    uint8_t year;
    uint8_t century;
} CmosReading;

CmosReading cmos_read();
