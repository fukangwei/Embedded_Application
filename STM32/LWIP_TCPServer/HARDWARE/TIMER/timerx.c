#include "timerx.h"
#include "led.h"
#include "usart.h"

u32 lwip_timer = 0;

void TIM6_IRQHandler ( void ) {
    if ( TIM_GetITStatus ( TIM6, TIM_IT_Update ) != RESET ) {
        lwip_timer++;
    }

    TIM_ClearITPendingBit ( TIM6, TIM_IT_Update  );
}

void TIM6_Int_Init ( u16 arr, u16 psc ) {
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB1PeriphClockCmd ( RCC_APB1Periph_TIM6, ENABLE );
    TIM_TimeBaseStructure.TIM_Period = arr;
    TIM_TimeBaseStructure.TIM_Prescaler = psc;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit ( TIM6, &TIM_TimeBaseStructure );
    TIM_ITConfig ( TIM6, TIM_IT_Update | TIM_IT_Trigger, ENABLE );
    TIM_Cmd ( TIM6, ENABLE );
    NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init ( &NVIC_InitStructure );
}