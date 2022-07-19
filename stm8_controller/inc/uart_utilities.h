//---------------------------------------------------------------------------
//
//	Raspberry Pi SuperUPS
//
//	Copyright (C) 2020 Scott Baker
//	Copyright (C) 2022 akuker
//
//	[ Uart utilities ]
//
//---------------------------------------------------------------------------

#pragma once

#ifdef STM8_I2C_STDIO
#include "ringbuffer.h"

extern ring_buffer_t stdout_ringbuffer;
#endif

int putchar(int c);
void uart_init();
