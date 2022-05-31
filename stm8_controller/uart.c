/* Simple "Hello World" UART output  */
#include <string.h>
#include <stdint.h>
#include "stm8s.h"

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

    while(1) {
        uart_write("Hello World!\r\n");
        delay(400000L);
    }
}
