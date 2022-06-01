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
    // VIN_LOW = (uint8_t)2,
    VIN_HIGH = (uint8_t)3,
    // VUPS_LOW = (uint8_t)4,
    VUPS_HIGH = (uint8_t)5,
    MOSFET = (uint8_t)6,
    ON_THRESH = (uint8_t)7,
    OFF_THRESH = (uint8_t)8,
    COUNTDOWN = (uint8_t)9,
    STATE = (uint8_t)10,
    CYCLE_DELAY = (uint8_t)11,
    FAIL_SHUTDOWN_DELAY = (uint8_t)12,
    RUN_COUNTER = (uint8_t)13,
    POWERUP_THRESH = (uint8_t)14,
    SHUTDOWN_THRESH = (uint8_t)15,
    R1_VALUE = (uint8_t)16,
    R2_VALUE = (uint8_t)17,
    REGISTERS_SIZE = (uint8_t)18, // Must be the last entry
} i2c_register_index;

extern volatile uint8_t i2c_register_values[REGISTERS_SIZE];
