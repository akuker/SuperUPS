/*
 *
 *
 *
 * 
 * UART example based upon jukkas example:
 *     https://github.com/jukkas/stm8-sdcc-examples
 * 
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stm8s.h>
#include <stm8s_adc1.h>
#include "ups_state_machine.h"
#include "i2c_slave.h"
#include "i2c_register_data.h"
#include "ups_constants.h"

/* Simple busy loop delay */
void delay(unsigned long count) {
    while (count--)
        nop();
}

int putchar (int c)
{
  /* Write a character to the UART1 */
  UART1_SendData8(c);
  /* Loop until the end of transmission */
  while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);
  return (c);
}

int uart_write(const char *str) {
    char i;
    for(i = 0; i < strlen(str); i++) {
        while(!(UART1->SR & UART1_SR_TXE)); // !Transmit data register empty
        UART1->DR = str[i];
    }
    return(i); // Bytes sent
}

int uart_writec(const char ch) {
    while(!(UART1->SR & UART1_SR_TXE)); // !Transmit data register empty
    UART1->DR = ch;
    return 1; // Bytes sent
}

void uart_write_uint16(uint16_t val) {
    (void)uart_writec(val/10000 + '0');
    (void)uart_writec((val % 10000) / 1000 + '0');
    (void)uart_writec((val % 1000)/100 + '0');
    (void)uart_writec((val % 100)/10 + '0');
    (void)uart_writec(val % 10 + '0');
    (void)uart_writec(' ');
}

void uart_write_uint8(uint8_t val) {
    (void)uart_writec((val % 1000)/100 + '0');
    (void)uart_writec((val % 100)/10 + '0');
    (void)uart_writec(val % 10 + '0');
    (void)uart_writec(' ');
}


uint16_t ADC_Read(ADC1_Channel_TypeDef);
extern u8 i2c_counter;

int main(void)
{
    /* Set clock to full speed (16 Mhz) */
    CLK->CKDIVR = 0;


    UART1_DeInit();
    UART1_Init(9600, UART1_WORDLENGTH_8D, 
                UART1_STOPBITS_1, UART1_PARITY_NO, 
                UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);
    printf("\n\rUART1 Example :retarget the C library printf()/getchar() functions to the UART\n\r");
    printf("\n\rEnter Text\n\r");

    uart_write("---------------- Starting up.... \r\n");


    // TODO: This doesn't work when the i2c controller is enabled....
    // GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_PP_HIGH_SLOW);

    Init_I2C();
    ups_init();
    
	/* Enable general interrupts */
	enableInterrupts();
    uint8_t counter = 0;

    while(1) {
        uint16_t result_AIN3 = ADC_Read(ADC1_CHANNEL_4);
        printf("%04X ", result_AIN3);
        // uart_write_uint16(result_AIN3);
        
        result_AIN3 = ADC_Read(ADC1_CHANNEL_3);
        printf("%04X ", result_AIN3);
        // uart_write_uint16(result_AIN3);

        i2c_register_values[VIN_HIGH] = (uint8_t)(ADC_Read(ADC1_CHANNEL_3) >> 8);
        i2c_register_values[VUPS_HIGH] = (uint8_t)(ADC_Read(ADC1_CHANNEL_4) >> 8);

        uart_writec(counter + '0');
        counter++;
        if(counter > 9)
            counter = 0;
        uart_writec(' ');
        uart_write_uint8(i2c_counter);
        uart_write(" Hello World!\r\n");
        delay(100000L);
        GPIO_WriteLow(GPIOB, GPIO_PIN_5);
        delay(100000L);
        GPIO_WriteHigh(GPIOB, GPIO_PIN_5);
    }
}
