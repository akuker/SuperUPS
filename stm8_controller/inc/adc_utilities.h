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

#pragma once

#include "i2c_register_data.h"

extern uint16_t raw_adc_values[I2C_REGISTERS_SIZE];

void adc_init(void);
void adc_step(void);