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

#include <stdint.h>

int putchar(int c);
int uart_write(const char *str);
int uart_writec(const char ch);
void uart_write_uint16(uint16_t val);
void uart_write_uint8(uint8_t val);

void uart_init();