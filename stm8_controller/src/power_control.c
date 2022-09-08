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

#include "power_control.h"
#include <stdint.h>
#include <stm8s.h>
#include "i2c_register_data.h"
#include "timer4_utilities.h"

// Record the current state of the mosfet, so that we
// only update the gpio when it changes.
static uint8_t current_power_state = 0xFF;

inline void power_off()
{
    GPIO_WriteLow(GPIOC, GPIO_PIN_3);
    current_power_state = 0;
}

inline void power_on()
{
    GPIO_WriteHigh(GPIOC, GPIO_PIN_3);
    current_power_state = 1;
}

void power_control_init()
{
    i2c_register_values[I2C_MOSFET_ENABLE] = 0;
    GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_OD_LOW_FAST);
    power_off();
}

void pwr_ctrl_step()
{

    // If we're in test mode, we always want to enable power to the host
    if (i2c_register_values[I2C_TEST_MODE_ENABLE] != 0)
    {
        power_on();
        return;
    }

    // check if the state has changed, then update the mosfet state
    if (current_power_state != i2c_register_values[I2C_MOSFET_ENABLE])
    {
        if (i2c_register_values[I2C_MOSFET_ENABLE] != 0)
        {
            // low will turn the mosfet on
            power_on();

#if 0
            // The following feature was part of Dr. Baker's original design.
            // However, it is not necessary with the SuperUPS design. The 
            // TPS54331 buck converter has a built-in "slow start" function
            // that will control inrush current
            //--------------------------------------
            // Slow-start the mosfet.
            // Otherwise, the sudden current inrush will cause the dc/dc
            // converter to trip into overcurrent mode and shut off.
            for (uint8_t i = 0; i < 200; i++)
            {
                power_on();
                delay_us(10);
                power_off();
                delay_us(10);
            }
            for (uint8_t i = 0; i < 200; i++)
            {
                power_on();
                delay_us(20);
                power_off();
                delay_us(10);
            }
            power_on();
#endif
        }
        else
        {
            // high will turn the mosfet off
            power_off();
        }
    }
}
