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
#include "mosfet_handler.h"

#define R1 100 // 10k
#define R2 22  // 2.2k

extern u8 i2c_counter;

static const char STATE_DISABLED_STRING[] = "STATE_DISABLED";
static const char STATE_WAIT_OFF_STRING[] = "STATE_WAIT_OFF";
static const char STATE_WAIT_ON_STRING[] = "STATE_WAIT_ON";
static const char STATE_POWERUP_STRING[] = "STATE_POWERUP";
static const char STATE_RUNNING_STRING[] = "STATE_RUNNING";
static const char STATE_FAIL_SHUTDOWN_STRING[] = "STATE_FAIL_SHUTDOWN";
static const char STATE_FAIL_SHUTDOWN_DELAY_STRING[] = "STATE_FAIL_SHUTDOWN_DELAY";
static const char STATE_CYCLE_DELAY_STRING[] = "STATE_CYCLE_DELAY";

const char *states_strings[] = {
    STATE_DISABLED_STRING,
    STATE_WAIT_OFF_STRING,
    STATE_WAIT_ON_STRING,
    STATE_POWERUP_STRING,
    STATE_RUNNING_STRING,
    STATE_FAIL_SHUTDOWN_STRING,
    STATE_FAIL_SHUTDOWN_DELAY_STRING,
    STATE_CYCLE_DELAY_STRING,
};

static uint8_t counter = 0;

void debug_output()
{
    printf("%s ", states_strings[i2c_register_values[STATE]]);
    printf("VIN:%08X %d ", i2c_register_values[VIN_HIGH], i2c_register_values[VIN_HIGH]);
    printf("VUPS:%08X %d ", i2c_register_values[VUPS_HIGH], i2c_register_values[VUPS_HIGH]);
    printf("VOUT:%08X %d ", i2c_register_values[VOUT_HIGH], i2c_register_values[VOUT_HIGH]);
    printf("Mosfet: %d ", i2c_register_values[MOSFET]);

    uart_writec(counter + '0');
    counter++;
    if (counter > 9)
        counter = 0;
    printf(" run count:%d i2c reads: %d\n\r", counter, i2c_counter);
}

int main(void)
{
    /* Set clock to full speed (16 Mhz) */
    CLK->CKDIVR = 0;

    mosfet_init();
    uart_init();
    i2c_init();
    ups_init();
    adc_init();

    /* Enable general interrupts */
    enableInterrupts();
    adc_step();
    debug_output();

    while (1)
    {
        adc_step();
        debug_output();
        ups_step();
        handle_mosfet();
        debug_output();
        delay(100000L);
    }
}
