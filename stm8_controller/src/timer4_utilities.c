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

#include <stdint.h>
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
