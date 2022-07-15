//---------------------------------------------------------------------------
//
//	Raspberry Pi SuperUPS
//
//	Copyright (C) 2020 Scott Baker
//	Copyright (C) 2022 akuker
//
//	[ I2C data array ]
//
//  This is the "master" copy of the information that can be requested by
//  the I2C master. The I2C interrupt routine will read the data from this
//  array and return it to the master.
//---------------------------------------------------------------------------
#pragma once

#include <stdint.h>

typedef enum
{
    I2C_CHARACTER_BUFFER = (uint8_t)1,
    I2C_TEST_MODE_ENABLE = (uint8_t)2,
    I2C_ADC_VOLTAGE_IN = (uint8_t)3,
    I2C_ADC_VOLTAGE_OUT = (uint8_t)4,
    I2C_ADC_SUPER_CAP_VOLTAGE = (uint8_t)5,
    I2C_MOSFET_ENABLE = (uint8_t)6,
    I2C_12V_ON_THRESH = (uint8_t)7,
    I2C_12V_OFF_THRESH = (uint8_t)8,
    I2C_COUNTDOWN = (uint8_t)9,
    I2C_CURENT_STATE = (uint8_t)10,
    I2C_CYCLE_DELAY = (uint8_t)11,
    I2C_FAIL_SHUTDOWN_DELAY = (uint8_t)12,
    I2C_RUN_COUNTER = (uint8_t)13,
    I2C_UPS_POWERUP_THRESH = (uint8_t)14,
    I2C_SHUTDOWN_THRESH = (uint8_t)15,
    I2C_VOLTAGE_DIVIDER_R1_VALUE = (uint8_t)16,
    I2C_VOLTAGE_DIVIDER_R2_VALUE = (uint8_t)17,
    I2C_INTERFACE_VERSION = (uint8_t)18,
    I2C_BUILD_VERSION = (uint8_t)19,
    I2C_DEVICE_ID = (uint8_t)20,
    I2C_POWER_BUTTON_STATE = (uint8_t)21,
    I2C_AUX_BUTTON_STATE = (uint8_t)22,
    I2C_REGISTERS_SIZE = (uint8_t)23 // Must be the last entry
} i2c_register_index;

extern volatile uint8_t i2c_register_values[I2C_REGISTERS_SIZE];

void init_register_data();
