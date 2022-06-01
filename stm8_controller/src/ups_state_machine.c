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

inline void mosfetOff()
{
// #ifdef DCDC_INSTEAD_OF_MOSFET
//     // to turn the DCDC off, pull down the enable
//     PORTB &= (~PIN_MOSFET);
//     DDRB |= PIN_MOSFET;
// #else
//     PORTB |= PIN_MOSFET;
// #endif
}

inline void mosfetOn()
{
// #ifdef DCDC_INSTEAD_OF_MOSFET
//     // to turn the DCDC off, float the enable
//     DDRB &= (~PIN_MOSFET);
// #else
//     PORTB &= (~PIN_MOSFET);
// #endif
}

static void handleRegMosfet()
{
//     if (i2c_register_values[MOSFET]) {
//         // low will turn the mosfet on
//         // mosfetOn()

// #ifdef SLOW_START_MOSFET
//         // Slow-start the mosfet.
//         // Otherwise, the sudden current inrush will cause the dc/dc
//         // converter to trip into overcurrent mode and shut off.
//         uint8_t i;
//         for (i=0; i<200; i++) {
//             mosfetOn();
//             delayMicroseconds(3);
//             mosfetOff();
//             delayMicroseconds(3);
//         }
//         for (i=0; i<200; i++) {
//             mosfetOn();
//             delayMicroseconds(6);
//             mosfetOff();
//             delayMicroseconds(3);
//         }
// #endif
//         mosfetOn();
//     } else {
//         // high will turn the mosfet off
//         mosfetOff();
//     }
}


void ups_init() {

    mosfetOff();
    VAL_STATE = STATE_POWERUP;
}

void ups_step() {

    switch (VAL_STATE) {
        case STATE_DISABLED:
            // state machine is disabled -- do nothing
            break;

        case STATE_WAIT_OFF:
            if (VAL_VIN_HIGH <= VAL_OFF_THRESH) {
                VAL_STATE = STATE_WAIT_ON;
            }
            break;

        case STATE_WAIT_ON:
            if (VAL_VIN_HIGH >= VAL_ON_THRESH) {
                VAL_STATE = STATE_POWERUP;
            }
            break;

        case STATE_FAIL_SHUTDOWN:
        case STATE_POWERUP:
            if ((VAL_VIN_HIGH >= VAL_ON_THRESH) && (VAL_VUPS_HIGH >= VAL_POWERUP_THRESH)) {
                VAL_COUNTDOWN = 0;
                VAL_MOSFET = 1;
                handleRegMosfet();
                VAL_STATE = STATE_RUNNING;
            }
            break;

        case STATE_RUNNING:
            if ((VAL_VIN_HIGH <= VAL_OFF_THRESH) && (VAL_VUPS_HIGH <= VAL_OFF_THRESH)) {
                VAL_MOSFET = 0;
                handleRegMosfet();
                VAL_FAIL_SHUTDOWN_DELAY = 10;
                VAL_STATE = STATE_FAIL_SHUTDOWN_DELAY;
            }
            if (VAL_COUNTDOWN > 0) {
                if ((VAL_RUN_COUNTER%10)==0) {
                    VAL_COUNTDOWN --;
                }
                if (VAL_COUNTDOWN == 0) {
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
            if (VAL_FAIL_SHUTDOWN_DELAY > 0) {
                VAL_FAIL_SHUTDOWN_DELAY--;
            }
            if (VAL_FAIL_SHUTDOWN_DELAY==0) {
                VAL_STATE = STATE_FAIL_SHUTDOWN;
            }

        case STATE_CYCLE_DELAY:
            // shutdown due to power-off request; delay so the pi can safely power cycle
            if (VAL_CYCLE_DELAY>0) {
                VAL_CYCLE_DELAY--;
            }
            if (VAL_CYCLE_DELAY==0) {
                VAL_STATE = STATE_WAIT_OFF;
            }
            break;
    }

    VAL_RUN_COUNTER++;

    // tws_delay(10); // delay 10ms
}