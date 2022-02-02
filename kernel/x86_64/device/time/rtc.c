#include "rtc.h"
#include "stdint.h"
#include "common/device/port.h"
#include "interrupts.h"

// === PRIVATE FUNCTIONS ========================

void read_from_cmos(uint8_t array []) {
   uint8_t tvalue, index;
 
   for(index = 0; index < 128; index++) {
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
cmosReading cmos_read() {
    uint8_t cmos[128];
    read_from_cmos(cmos);

    cmosReading result;
    itoa(cmos[0] + cmos[1], 16, result.second);
    itoa(cmos[2] + cmos[3], 16, result.minute);
    itoa(cmos[4] + cmos[5], 16, result.hour); 
    itoa(cmos[6], 16, result.weekday);
    itoa(cmos[7], 16, result.monthday);
    itoa(cmos[8], 16, result.month);
    itoa(cmos[9], 16, result.year);
    itoa(cmos[50], 16, result.century);
    
    return result;
}