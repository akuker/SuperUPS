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

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stm8s_uart1.h>
#include "uart_utilities.h"

int putchar(int c)
{
    /* Write a character to the UART1 */
    UART1_SendData8(c);
    /* Loop until the end of transmission */
    while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET)
        ;
    return (c);
}

int uart_write(const char *str)
{
    char i;
    for (i = 0; i < strlen(str); i++)
    {
        while (!(UART1->SR & UART1_SR_TXE))
            ; // !Transmit data register empty
        UART1->DR = str[i];
    }
    return (i); // Bytes sent
}

int uart_writec(const char ch)
{
    while (!(UART1->SR & UART1_SR_TXE))
        ; // !Transmit data register empty
    UART1->DR = ch;
    return 1; // Bytes sent
}

void uart_write_uint16(uint16_t val)
{
    (void)uart_writec(val / 10000 + '0');
    (void)uart_writec((val % 10000) / 1000 + '0');
    (void)uart_writec((val % 1000) / 100 + '0');
    (void)uart_writec((val % 100) / 10 + '0');
    (void)uart_writec(val % 10 + '0');
    (void)uart_writec(' ');
}

void uart_write_uint8(uint8_t val)
{
    (void)uart_writec((val % 1000) / 100 + '0');
    (void)uart_writec((val % 100) / 10 + '0');
    (void)uart_writec(val % 10 + '0');
    (void)uart_writec(' ');
}

void uart_init()
{
    UART1_DeInit();
    UART1_Init(9600, UART1_WORDLENGTH_8D,
               UART1_STOPBITS_1, UART1_PARITY_NO,
               UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);
    printf("\n\rUART1 Example :retarget the C library printf()/getchar() functions to the UART\n\r");
    printf("\n\rEnter Text\n\r");

    uart_write("---------------- Starting up.... \r\n");
}