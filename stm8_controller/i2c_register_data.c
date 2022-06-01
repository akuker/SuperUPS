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

#include "i2c_register_data.h"
#include "ups_constants.h"

volatile uint8_t i2c_register_values[] =
{
    0x0,        // reserveds
    0x1,        // reserved
    0x0,        // vin - low (reserved)
    0x0,        // vin - high
    0x0,        // vups - low (reserved)
    0x0,        // vups - high
    0x0,        // mosfet switch state
    ON_THRESH,  // on threshold
    OFF_THRESH, // off threshold
    0x0,        // countdown in 100ms units
    STATE_POWERUP,   // state
    0x0,        // cycle delay
    0x0,        // fail-shutdown delay
    0x0,        // run counter
    POWERUP_THRESH, // powerup threshold
    SHUTDOWN_THRESH, // software shutdown threshold
    0x0,     // R1 resistor value
    0x0,     // R2 resistor value
};