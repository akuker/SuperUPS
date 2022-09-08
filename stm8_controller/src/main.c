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
#include "gpio_handler.h"
#include "i2c_slave.h"
#include "i2c_register_data.h"
#include "uart_utilities.h"
#include "ups_constants.h"
#include "ups_state_machine.h"
#include "power_control.h"
#include "timer4_utilities.h"

extern u8 i2c_counter;

static const char STATE_DISABLED_STRING[] = "STATE_DISABLED";
static const char STATE_WAIT_OFF_STRING[] = "STATE_WAIT_OFF";
static const char STATE_WAIT_ON_STRING[] = "STATE_WAIT_ON";
static const char STATE_POWERUP_STRING[] = "STATE_POWERUP";
static const char STATE_RUNNING_STRING[] = "STATE_RUNNING";
static const char STATE_FAIL_SHUTDOWN_STRING[] = "STATE_POWER_LOSS_SHUTDOWN";
static const char STATE_FAIL_SHUTDOWN_DELAY_STRING[] = "STATE_POWER_LOSS_SHUTDOWN_DELAY";
static const char STATE_CYCLE_DELAY_STRING[] = "STATE_COMMANDED_POWER_CYCLE_DELAY";

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

void debug_output()
{
    static ups_state_type prev_state = STATE_DISABLED;
    static uint8_t prev_test_mode = 0xFF;

    if(i2c_register_values[I2C_CURENT_STATE] != prev_state){
        printf("Changed to %s \n\r", states_strings[i2c_register_values[I2C_CURENT_STATE]]);
        prev_state = i2c_register_values[I2C_CURENT_STATE];
    }
    if(i2c_register_values[I2C_TEST_MODE_ENABLE] != prev_test_mode){
        printf("Test mode changed to %d\n\r", i2c_register_values[I2C_TEST_MODE_ENABLE]);
        prev_test_mode = i2c_register_values[I2C_TEST_MODE_ENABLE];
        if(i2c_register_values[I2C_TEST_MODE_ENABLE]){
            print_build_date();
        }
    }
}

static uint16_t counter = 0;

int main(void)
{
    /* Set clock to full speed (16 Mhz) */
    CLK->CKDIVR = 0;

    init_register_data();
    power_control_init();
    uart_init();
    i2c_init();
    ups_init();
    adc_init();
    gpios_init();

    /* Enable general interrupts */
    enableInterrupts();
    adc_step();
    debug_output();

    while (1)
    {
        adc_step();
        // Only update the UPS state machine approximately every 100ms
        counter++;
        if(counter > 99){
            ups_step();
            counter = 0;
        }
        pwr_ctrl_step();
        gpios_step();
        debug_output();
        delay_ms(1);
    }
}
