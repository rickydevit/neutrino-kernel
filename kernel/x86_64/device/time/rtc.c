#include "rtc.h"
#include <stdint.h>
#include <stdbool.h>
#include <neutrino/time.h>
#include <string.h>
#include "kernel/common/device/port.h"
#include "interrupts.h"

// === PRIVATE FUNCTIONS ========================

void read_from_cmos(uint8_t array[]) {
   uint8_t tvalue, index;
 
   for(index = 0; index < 16; index++) {
       disable_interrupts();
       port_byte_out(0x70, index);
       tvalue = port_byte_in(0x71);
       enable_interrupts();
 
       array[index] = tvalue;
   }
}

// === PUBLIC FUNCTIONS =========================

// *Return the CMOS reading
// @return the reading value of the CMOS
DateTime cmos_read() {
    uint8_t cmos[16];

    do {
        read_from_cmos(cmos);
    } while ((cmos[0xa] & 0x80) == 0);
        
    return (DateTime) {
        .second = cmos[0] | (uint16_t)(cmos[1] << 8),
        .minute = cmos[2] | (uint16_t)(cmos[3] << 8),
        .hour = cmos[4] | (uint16_t)(cmos[5] << 8),
        .day = cmos[7],
        .month = cmos[8],
        .year = cmos[9] + 2000,
    };
}