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
#include <stdint.h>
#include <stm8s.h>
#include <stm8s_adc1.h>
#include "ups_state_machine.h"

/* Simple busy loop delay */
void delay(unsigned long count) {
    while (count--)
        nop();
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

uint16_t ADC_Read(ADC1_Channel_TypeDef);

int main(void)
{
    /* Set clock to full speed (16 Mhz) */
    CLK->CKDIVR = 0;

    // Setup UART1 (TX=D5)
    UART1->CR2 |= UART1_CR2_TEN; // Transmitter enable
    // UART1_CR2 |= UART_CR2_REN; // Receiver enable
    UART1->CR3 &= ~(UART1_CR3_STOP); // 1 stop bit
    // 9600 baud: UART_DIV = 16000000/9600 ~ 1667 = 0x0683
    UART1->BRR2 = 0x03; UART1->BRR1 = 0x68; // 0x0683 coded funky way (see ref manual)

    GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_OD_LOW_SLOW);

    ups_init();

    uint8_t counter = 0;

    while(1) {
        uint16_t result_AIN3 = ADC_Read(ADC1_CHANNEL_4);

        uart_writec(result_AIN3/10000 + '0');
        uart_writec((result_AIN3 % 10000) / 1000 + '0');
        uart_writec((result_AIN3 % 1000)/100 + '0');
        uart_writec((result_AIN3 % 100)/10 + '0');
        uart_writec(result_AIN3 % 10 + '0');
        uart_writec(' ');
        
        result_AIN3 = ADC_Read(ADC1_CHANNEL_3);
        uart_writec(result_AIN3/10000 + '0');
        uart_writec((result_AIN3 % 10000) / 1000 + '0');
        uart_writec((result_AIN3 % 1000)/100 + '0');
        uart_writec((result_AIN3 % 100)/10 + '0');
        uart_writec(result_AIN3 % 10 + '0');
        uart_writec(' ');

        uart_writec(counter + '0');
        counter++;
        if(counter > 9)
            counter = 0;
        uart_write(" Hello World!\r\n");
        delay(100000L);
        GPIO_WriteLow(GPIOB, GPIO_PIN_5);
        delay(100000L);
        GPIO_WriteHigh(GPIOB, GPIO_PIN_5);
    }
}
