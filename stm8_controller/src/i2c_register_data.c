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

volatile uint8_t i2c_register_values[I2C_REGISTERS_SIZE] = {0};

// The ratio between R1 and R2 is what matters. These act as
// a voltage divider
#define R1 100 // 10k
#define R2 22  // 2.2k

void init_register_data()
{

    // To convert the voltages...
    //    (Voltage * (R2 / (R1+R2)))/3.3v * 0xFF
    i2c_register_values[I2C_12V_ON_THRESH] = 0xA0;      // 11.5v
    i2c_register_values[I2C_12V_OFF_THRESH] = 0x93;     // 10.5v
    i2c_register_values[I2C_UPS_POWERUP_THRESH] = 0x70; // 8.0v
    i2c_register_values[I2C_CURENT_STATE] = STATE_POWERUP;
    i2c_register_values[I2C_VOLTAGE_DIVIDER_R1_VALUE] = R1;
    i2c_register_values[I2C_VOLTAGE_DIVIDER_R2_VALUE] = R2;
    i2c_register_values[I2C_TEST_MODE_ENABLE] = 0;
    i2c_register_values[I2C_INTERFACE_VERSION] = 1;
}