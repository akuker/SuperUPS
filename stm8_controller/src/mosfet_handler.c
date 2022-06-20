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
#include "timer4_utilities.h"

// Record the current state of the mosfet, so that we
// only update the gpio when it changes.
static uint8_t current_mosfet_state = 0xFF;

inline void mosfetOff()
{
    GPIO_WriteHigh(GPIOC, GPIO_PIN_3);
    current_mosfet_state = 0;
}

inline void mosfetOn()
{
    GPIO_WriteLow(GPIOC, GPIO_PIN_3);
    current_mosfet_state = 1;
}

void mosfet_init()
{
    i2c_register_values[I2C_MOSFET_ENABLE] = 0;
    GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_OD_LOW_FAST);
    mosfetOff();
}

void handle_mosfet()
{

    // If we're in test mode, we always want to enable power to the host
    if (i2c_register_values[I2C_TEST_MODE_ENABLE])
    {
        mosfetOn();
        return;
    }

    // check if the state has changed, then update the mosfet state
    if (current_mosfet_state != i2c_register_values[I2C_MOSFET_ENABLE])
    {
        if (i2c_register_values[I2C_MOSFET_ENABLE])
        {
            // low will turn the mosfet on
            mosfetOn();

            // Slow-start the mosfet.
            // Otherwise, the sudden current inrush will cause the dc/dc
            // converter to trip into overcurrent mode and shut off.
            for (uint8_t i = 0; i < 200; i++)
            {
                mosfetOn();
                delay_us(10);
                mosfetOff();
                delay_us(10);
            }
            for (uint8_t i = 0; i < 200; i++)
            {
                mosfetOn();
                delay_us(20);
                mosfetOff();
                delay_us(10);
            }
            mosfetOn();
        }
        else
        {
            // high will turn the mosfet off
            mosfetOff();
        }
    }
}
