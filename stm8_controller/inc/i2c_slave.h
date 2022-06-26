//---------------------------------------------------------------------------
//
//	Raspberry Pi SuperUPS
//
//  Copyright (C) 2009 STMicroelectronics
//	Copyright (C) 2020 Scott Baker
//	Copyright (C) 2022 akuker
//
//	[ I2C slave handler ]
//
//  The I2C slave handling is based upon STMicroelectronics AN3281
//     https://www.st.com/en/embedded-software/stsw-stm8004.html
//
//  Includes the routines for handling the I2C requests from the master.
//  The i2c_slave_check_event function is registered in the interrupt table
//  and will be called when there is an I2C transaction for this slave
//  device (based upon the address).
//
//---------------------------------------------------------------------------

#pragma once
#include "stm8s.h"
#include "stm8s_itc.h"

#define SLAVE_ADDRESS 0x51

void i2c_init(void);
// Different compilers have different syntax for registering an interrupt
#ifdef _RAISONANCE_
void I2C_Slave_check_event(void) interrupt ITC_IRQ_I2C;
#elseif _COSMIC_
@far @interrupt void I2C_Slave_check_event(void);
#else
void I2C_Slave_check_event(void) __interrupt(ITC_IRQ_I2C);
#endif