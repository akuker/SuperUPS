//---------------------------------------------------------------------------
//
//	Raspberry Pi SuperUPS
//
//	Copyright (C) 2020 Scott Baker
//	Copyright (C) 2022 akuker
//
//	[ SuperUPS master state machine ]
//
//---------------------------------------------------------------------------

#include <stdint.h>
#include <stm8s.h>
#include "ups_constants.h"
#include "i2c_register_data.h"

inline void mosfetOff()
{
    GPIO_WriteHigh(GPIOC, GPIO_PIN_3);
}

inline void mosfetOn()
{
    GPIO_WriteLow(GPIOC, GPIO_PIN_3);
}
void delay(unsigned long count);
void handleRegMosfet()
{
    static uint8_t prev_state = 0xFF;

    if (prev_state != i2c_register_values[MOSFET])
    {
        if (i2c_register_values[MOSFET])
        {
            // low will turn the mosfet on
            mosfetOn();

            // Slow-start the mosfet.
            // Otherwise, the sudden current inrush will cause the dc/dc
            // converter to trip into overcurrent mode and shut off.
            uint8_t i;
            for (i = 0; i < 200; i++)
            {
                mosfetOn();
                delay(10);
                mosfetOff();
                delay(10);
            }
            for (i = 0; i < 200; i++)
            {
                mosfetOn();
                delay(20);
                mosfetOff();
                delay(10);
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

void ups_init()
{

    GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_OD_LOW_FAST);
    mosfetOff();
    VAL_STATE = STATE_POWERUP;
}

void ups_step()
{

    switch (VAL_STATE)
    {
    case STATE_DISABLED:
        // state machine is disabled -- do nothing
        break;

    case STATE_WAIT_OFF:
        if (VAL_VIN_HIGH <= VAL_OFF_THRESH)
        {
            VAL_STATE = STATE_WAIT_ON;
        }
        break;

    case STATE_WAIT_ON:
        if (VAL_VIN_HIGH >= VAL_ON_THRESH)
        {
            VAL_STATE = STATE_POWERUP;
        }
        break;

    case STATE_FAIL_SHUTDOWN:
    case STATE_POWERUP:
        if ((VAL_VIN_HIGH >= VAL_ON_THRESH) && (VAL_VUPS_HIGH >= VAL_POWERUP_THRESH))
        {
            VAL_COUNTDOWN = 0;
            VAL_MOSFET = 1;
            handleRegMosfet();
            VAL_STATE = STATE_RUNNING;
        }
        break;

    case STATE_RUNNING:
        if ((VAL_VIN_HIGH <= VAL_OFF_THRESH) && (VAL_VUPS_HIGH <= VAL_OFF_THRESH))
        {
            VAL_MOSFET = 0;
            handleRegMosfet();
            VAL_FAIL_SHUTDOWN_DELAY = 10;
            VAL_STATE = STATE_FAIL_SHUTDOWN_DELAY;
        }
        if (VAL_COUNTDOWN > 0)
        {
            if ((VAL_RUN_COUNTER % 10) == 0)
            {
                VAL_COUNTDOWN--;
            }
            if (VAL_COUNTDOWN == 0)
            {
                // turn off the mosfet and wait for powercycle before
                // turning back on.
                VAL_CYCLE_DELAY = 10;
                VAL_STATE = STATE_CYCLE_DELAY;
                VAL_MOSFET = 0;
                handleRegMosfet();
            }
        }
        break;

    case STATE_FAIL_SHUTDOWN_DELAY:
        // shutdown due to power failure; delay so the pi can safely power cycle
        if (VAL_FAIL_SHUTDOWN_DELAY > 0)
        {
            VAL_FAIL_SHUTDOWN_DELAY--;
        }
        if (VAL_FAIL_SHUTDOWN_DELAY == 0)
        {
            VAL_STATE = STATE_FAIL_SHUTDOWN;
        }

    case STATE_CYCLE_DELAY:
        // shutdown due to power-off request; delay so the pi can safely power cycle
        if (VAL_CYCLE_DELAY > 0)
        {
            VAL_CYCLE_DELAY--;
        }
        if (VAL_CYCLE_DELAY == 0)
        {
            VAL_STATE = STATE_WAIT_OFF;
        }
        break;
    }

    VAL_RUN_COUNTER++;

    // tws_delay(10); // delay 10ms
}