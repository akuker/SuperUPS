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

#include "ups_constants.h"
#include "i2c_register_data.h"

void ups_init()
{
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
            VAL_STATE = STATE_RUNNING;
        }
        break;

    case STATE_RUNNING:
        if ((VAL_VIN_HIGH <= VAL_OFF_THRESH) && (VAL_VUPS_HIGH <= VAL_OFF_THRESH))
        {
            VAL_MOSFET = 0;
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

}