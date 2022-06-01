//---------------------------------------------------------------------------
//
//	Raspberry Pi SuperUPS
//
//	Copyright (C) 2020 Scott Baker
//	Copyright (C) 2022 akuker
//
//	[ SuperUPS main ]
//
//  UART example based upon jukkas example:
//     https://github.com/jukkas/stm8-sdcc-examples
//
//---------------------------------------------------------------------------

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "adc_utilities.h"
#include "i2c_slave.h"
#include "i2c_register_data.h"
#include "uart_utilities.h"
#include "ups_constants.h"
#include "ups_state_machine.h"

/* Simple busy loop delay */
void delay(unsigned long count)
{
    while (count--)
        nop();
}

extern u8 i2c_counter;
void handleRegMosfet();

int main(void)
{
    /* Set clock to full speed (16 Mhz) */
    CLK->CKDIVR = 0;

    // TODO: This doesn't work when the i2c controller is enabled....
    // GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_PP_HIGH_SLOW);
    // GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_HIGH_SLOW);

    

    uart_init();
    i2c_init();
    ups_init();
    adc_init();

    /* Enable general interrupts */
    enableInterrupts();
    uint8_t counter = 0;

    while (1)
    {
        adc_step();
        printf("VIN:%04X ", i2c_register_values[VIN_HIGH]);
        printf("VUPS:%04X ", i2c_register_values[VUPS_HIGH]);
        printf("VOUT:%04X ", i2c_register_values[VOUT_HIGH]);

        uart_writec(counter + '0');
        counter++;
        if (counter > 9)
            counter = 0;
        uart_writec(' ');
        uart_write_uint8(i2c_counter);
        uart_write(" Hello World!\r\n");
        // i2c_register_values[MOSFET] = 0;
        // handleRegMosfet();
        // delay(100000L);
        i2c_register_values[MOSFET] = 1;
        handleRegMosfet();
        delay(100000L);
        // delay(100000L);
        // GPIO_WriteHigh(GPIOC, GPIO_PIN_3);
    }
}
