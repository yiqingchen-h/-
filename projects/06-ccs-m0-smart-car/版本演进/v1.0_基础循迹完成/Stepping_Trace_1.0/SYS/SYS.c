#include "SYS.h"

void delay_ms(unsigned int ms)
{   
    while(ms--)
    {
        // CPUCLK_FREQ -- 当前主频在"ti_msp_dl_config.h"中定义
        delay_cycles(CPUCLK_FREQ / 1000);
    }
}


void delay_us(unsigned int us)
{   
    while(us--)
    {
        // CPUCLK_FREQ -- 当前主频在"ti_msp_dl_config.h"中定义
        delay_cycles(CPUCLK_FREQ / 1000000);
    }
}
