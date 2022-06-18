//---------------------------------------------------------------------------
//
//	Raspberry Pi SuperUPS
//
//	Copyright (C) 2020 Scott Baker
//	Copyright (C) 2022 akuker
//
//	[ Mosfet Handler ]
//
//---------------------------------------------------------------------------

#include "mosfet_handler.h"
#include <stdint.h>
#include <stm8s.h>
#include "i2c_register_data.h"

/* Simple busy loop delay */
void delay(unsigned long count)
{
    while (count--)
        nop();
}

inline void mosfetOff()
{
    GPIO_WriteHigh(GPIOC, GPIO_PIN_3);
}

inline void mosfetOn()
{
    GPIO_WriteLow(GPIOC, GPIO_PIN_3);
}

void mosfet_init()
{
    i2c_register_values[MOSFET] = 0;
    GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_OD_LOW_FAST);
    mosfetOff();
}

void handle_mosfet()
{
    static uint8_t prev_state = 0xFF;

    if (prev_state != i2c_register_values[MOSFET])
    {
        if (i2c_register_values[MOSFET])
        {
            // low will turn the mosfet on
            mosfetOn();

            // TOOD: Not sure if this is needed for the 12v version...
            // // Slow-start the mosfet.
            // // Otherwise, the sudden current inrush will cause the dc/dc
            // // converter to trip into overcurrent mode and shut off.
            // uint8_t i;
            // for (i = 0; i < 200; i++)
            // {
            //     mosfetOn();
            //     delay(10);
            //     mosfetOff();
            //     delay(10);
            // }
            // for (i = 0; i < 200; i++)
            // {
            //     mosfetOn();
            //     delay(20);
            //     mosfetOff();
            //     delay(10);
            // }
            // mosfetOn();
        }
        else
        {
            // high will turn the mosfet off
            mosfetOff();
        }
    }
}
