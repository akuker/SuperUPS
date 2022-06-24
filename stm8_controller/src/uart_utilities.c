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

#include <stdio.h>
#include "stm8s_uart1.h"
#include "checksum.h"
#include "i2c_register_data.h"
#include "uart_utilities.h"

static const char build_date[] = __DATE__; // the format is "Jan  1 2000"
static const char build_time[] = __TIME__; // the format is "00:00:00"
static const char device_id_string[] = "SuperUPS controller";

// This is referenced by the printf function
int putchar(int c)
{
    /* Write a character to the UART1 */
    UART1_SendData8(c);
    /* Loop until the end of transmission */
    while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET)
        ;
    return (c);
}

uint8_t calculate_build_date_crc()
{
    uint8_t build_date_crc = 0x0;

    for (int i = 0; i < sizeof(build_date); i++)
    {
        build_date_crc = update_crc_8(build_date_crc, build_date[i]);
    }
    for (int i = 0; i < sizeof(build_time); i++)
    {
        build_date_crc = update_crc_8(build_date_crc, build_time[i]);
    }

    return build_date_crc;
}

// For the device ID, we just want some value to tell that we're actually talking
// to this device. So, we'll use the crc8 of the device name.
uint8_t calculate_device_id()
{
    uint8_t device_id_crc = 0x0;

    for (int i = 0; i < sizeof(device_id_string); i++)
    {
        device_id_crc = update_crc_8(device_id_crc, device_id_string[i]);
    }

    return device_id_crc;
}

void uart_init()
{
    UART1_DeInit();
    UART1_Init(9600, UART1_WORDLENGTH_8D,
               UART1_STOPBITS_1, UART1_PARITY_NO,
               UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);

    uint8_t build_date_crc8 = calculate_build_date_crc();
    uint8_t device_id_crc8 = calculate_device_id();

    printf("Build Date: %s %s CRC: %02X Device: %02X\n\r", build_date, build_time, build_date_crc8, device_id_crc8);

    i2c_register_values[I2C_BUILD_VERSION] = build_date_crc8;
    i2c_register_values[I2C_DEVICE_ID] = device_id_crc8;
}