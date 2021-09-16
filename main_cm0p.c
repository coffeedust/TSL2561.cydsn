
#include "project.h"
#include "tsl2561.h"
#include <stdio.h>

int main(void) {
    __enable_irq();
    //Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR);
    mUART_Start();
    TSL2561_TSL2561(TSL2561_ADDR_FLOAT);
    TSL2561_begin();
    TSL2561_setGain(TSL2561_GAIN_0X);
    TSL2561_setTiming(TSL2561_INTEGRATIONTIME_13MS);
    for(;;) {
        uint32_t lum = TSL2561_getFullLuminosity();
        uint16_t ir, full;
        ir = lum >> 16;
        full = lum & 0xFFFF;
        
        char str[255];
        sprintf(str, "IR: %d\t Full: %d\t Visible: %d\t Lux: %d\n",
            ir, full, full - ir, TSL2561_calculateLux(full, ir));
        mUART_PutString(str);
        CyDelay(100); // 1초 지연
    }
}
