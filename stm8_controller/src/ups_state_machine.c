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
#include <stdio.h>

static uint8_t *current_state = &i2c_register_values[I2C_CURENT_STATE];
static uint8_t *input_voltage = &i2c_register_values[I2C_ADC_VOLTAGE_IN];
static uint8_t *ups_voltage = &i2c_register_values[I2C_ADC_SUPER_CAP_VOLTAGE];
static uint8_t *ups_power_valid_threshold = &i2c_register_values[I2C_UPS_POWERUP_THRESH];
static uint8_t *on_voltage_threshold = &i2c_register_values[I2C_12V_ON_THRESH];
static uint8_t *off_voltage_threshold = &i2c_register_values[I2C_12V_OFF_THRESH];
static uint8_t *mosfet_enable = &i2c_register_values[I2C_MOSFET_ENABLE];

static uint8_t *power_loss_delay_counter = &i2c_register_values[I2C_FAIL_SHUTDOWN_DELAY];
static uint8_t *commanded_power_cycle_counter = &i2c_register_values[I2C_CYCLE_DELAY];

static uint8_t *commanded_power_cycle_countdown = &i2c_register_values[I2C_COUNTDOWN];
static uint8_t *run_counter = &i2c_register_values[I2C_RUN_COUNTER];

void ups_init()
{
    *current_state = STATE_POWERUP;
}

void ups_step()
{

    switch (*current_state)
    {
    case STATE_DISABLED:
        // state machine is disabled -- do nothing
        break;

    case STATE_WAIT_OFF:
        if (*input_voltage <= *off_voltage_threshold)
        {
            *current_state = STATE_WAIT_ON;
        }
        break;

    case STATE_WAIT_ON:
        if (*input_voltage >= *on_voltage_threshold)
        {
            *current_state = STATE_POWERUP;
        }
        break;

    case STATE_POWER_LOSS_SHUTDOWN:
    case STATE_POWERUP:
        // Wait for the input voltage to be on and the super capacitors to be charged
        if ((*input_voltage >= *on_voltage_threshold) && (*ups_voltage >= *ups_power_valid_threshold))
        {
            *commanded_power_cycle_countdown = 0;
            *mosfet_enable = 1;
            *current_state = STATE_RUNNING;
        }
        break;

    case STATE_RUNNING:
        // If the input voltage is off/low and the super capacitors voltage
        // is too low, start a countdown to shut off the power output
        if ((*input_voltage <= *off_voltage_threshold) && (*ups_voltage <= *ups_power_valid_threshold))
        {
            // function gets called about every 100ms. Delay 5s
            *power_loss_delay_counter = 50;
            *current_state = STATE_POWER_LOSS_SHUTDOWN_DELAY;
        }
        // Check to see if the host has commanded a power cycle
        if (*commanded_power_cycle_countdown > 0)
        {
            // the run_counter should be updating at 1ms
            // the commanded power cycle is specified in 10ms increments
            if ((*run_counter % 10) == 0)
            {
                *commanded_power_cycle_countdown--;
            }
            if (*commanded_power_cycle_countdown == 0)
            {
                // Give the Pi 5 seconds to shut down
                *commanded_power_cycle_counter = 50;
                *current_state = STATE_COMMANDED_POWER_CYCLE_DELAY;
            }
        }
        break;

    case STATE_POWER_LOSS_SHUTDOWN_DELAY:
        // delay so the pi can safely power cycle
        if (*power_loss_delay_counter > 0)
        {
            *power_loss_delay_counter = *power_loss_delay_counter - 1;
        }
        else
        {
            // shutdown due to power failure; 
            *current_state = STATE_POWER_LOSS_SHUTDOWN;
            *mosfet_enable = 0;
        }
        break;

    case STATE_COMMANDED_POWER_CYCLE_DELAY:
        // shutdown due to power-off request; delay so the pi can safely power cycle
        if (*commanded_power_cycle_counter > 0)
        {
            *commanded_power_cycle_counter = *commanded_power_cycle_countdown - 1;
        }
        if (*commanded_power_cycle_counter == 0)
        {
            *current_state = STATE_WAIT_OFF;
            *mosfet_enable = 0;
        }
        break;
    }

    *run_counter++;
}
