//---------------------------------------------------------------------------
//
//	Raspberry Pi SuperUPS
//
//	Copyright (C) 2020 Scott Baker
//	Copyright (C) 2022 akuker
//
//	[ Timer utilities ]
//
//---------------------------------------------------------------------------

#include "timer4_utilities.h"
#include "stm8s_tim4.h"


void delay_1us()
{
    TIM4_DeInit(); /* Time base configuration */

    // 16MHz / 1 = Timer running at 16MHz
    // 1 tick = 0.0625 uS
    //
    // Time Period = 1uS / 0.0625uS = 16
    // - Remove 1 tick to account for the setup/teardown time
    TIM4_TimeBaseInit(TIM4_PRESCALER_1, 15);

    /* Clear TIM4 Flag */
    TIM4_ClearFlag(TIM4_FLAG_UPDATE);
    
    /* Enable Update interrupt */
    TIM4_ITConfig(TIM4_IT_UPDATE, DISABLE);
    
    /* Enable TIM4 */
    TIM4_Cmd(ENABLE);

    /* Enable Update interrupt */
    TIM4_ITConfig(TIM4_IT_UPDATE, DISABLE);

    /* the Enable of TIM4 */
    TIM4_Cmd(ENABLE);

    /* wait for the timer to expire */
    while (SET != TIM4_GetFlagStatus(TIM4_FLAG_UPDATE));
}

void delay_1ms()
{
    TIM4_DeInit(); /* Time base configuration */

    // 16MHz / 128 = Timer running at 125kHz
    // 1 tick = 8 uS
    //
    // Time Period = 1mS / 8uS = 125
    // - Remove 1 tick to account for the setup/teardown time
    TIM4_TimeBaseInit(TIM4_PRESCALER_128, 124);

    /* Clear TIM4 Flag */
    TIM4_ClearFlag(TIM4_FLAG_UPDATE);
    
    /* Enable Update interrupt */
    TIM4_ITConfig(TIM4_IT_UPDATE, DISABLE);
    
    /* Enable TIM4 */
    TIM4_Cmd(ENABLE);

    /* Enable Update interrupt */
    TIM4_ITConfig(TIM4_IT_UPDATE, DISABLE);

    /* the Enable of TIM4 */
    TIM4_Cmd(ENABLE);

    /* wait for the timer to expire */
    while (SET != TIM4_GetFlagStatus(TIM4_FLAG_UPDATE));
}

void delay_ms(unsigned long count)
{
    for(uint32_t i=0; i<count; i++){
        delay_1ms();
    }
}

void delay_us(unsigned long count)
{
    for(uint32_t i=0; i<count; i++){
        delay_1us();
    }
}


// void delay_us(unsigned char time)
// {
//     TIM4_EGR |= (1 << 0);
//     TIM4_CNTR = 0;
//     TIM4_CR1 |= (1 << 0);
//     while (TIM4_CNTR < time)
//         ;
//     TIM4_CR1 &= ~(1 << 0);
//     TIM4_SR &= ~(1 << 0); // clear flag
// }

// void delay_ms(unsigned int timer)
// {
//     time *= 10;
//     while (time--)
//         ;
//     delay_us(100);
// }

// #define TIM4_PERIOD 124

// void Dely1ms(void)
// {
//     TIM4_DeInit(); /* Time base configuration */
//     // 8M/64=125000HZ
//     //(x + 1)/125000=0.001s = 1ms
//     // x = 124
//     TIM4_TimeBaseInit(TIM4_PRESCALER_64, TIM4_PERIOD);
//     /* the Clear of TIM4 Update In Flag */
//     TIM4_ClearFlag(TIM4_FLAG_UPDATE);
//     /* the Enable Update interrupt */
//     TIM4_ITConfig(TIM4_IT_UPDATE, the DISABLE);
//     /* the Enable of TIM4 */
//     TIM4_Cmd(the ENABLE);
//     the while (!the SET = TIM4_GetFlagStatus(TIM4_FLAG_UPDATE));
// }

// void DelyMs(uint32_t ms)
// {
//     while (ms--)
//         Dely1ms();
// }


// //////////////////////////////////////////////
// // https://gist.github.com/stecman/f748abea0332be1e41640fd25b5ca861
// //////////////////////////////////////////////

// void _delay_us(uint16_t microseconds) {
//     TIM4->PSCR = TIM4_PRESCALER_1; // Set prescaler

//     // Set count to approximately 1uS (clock/microseconds in 1 second)
//     // The -1 adjusts for other runtime costs of this function
//     TIM4->ARR = ((16000000L)/1000000) - 1;


//     TIM4->CR1 = TIM4_CR1_CEN; // Enable counter

//     for (; microseconds > 1; --microseconds) {
//         while ((TIM4->SR1 & TIM4_SR1_UIF) == 0);

//         // Clear overflow flag
//         TIM4->SR1 &= ~TIM4_SR1_UIF;
//     }
// }

// void _delay_ms(uint16_t milliseconds)
// {
//     while (milliseconds)
//     {
//         _delay_us(1000);
//         milliseconds--;
//     }
// }