#include "timerx.h"
#include "led.h"
#include "usart.h"
#include "stm32f10x.h"

unsigned long ms = 0;
extern unsigned long  dhcp_time;

void TIM2_IRQHandler ( void ) {
    if ( TIM_GetITStatus ( TIM2, TIM_IT_Update ) != RESET ) {
        ms++;

        if ( ms >= 1000 ) {
            ms = 0;
            dhcp_time++;
        }
    }

    TIM_ClearITPendingBit ( TIM2, TIM_IT_Update  );
}

void TIM2_Int_Init ( u16 arr, u16 psc ) {
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB1PeriphClockCmd ( RCC_APB1Periph_TIM2, ENABLE );
    TIM_TimeBaseStructure.TIM_Period = arr;
    TIM_TimeBaseStructure.TIM_Prescaler = psc;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit ( TIM2, &TIM_TimeBaseStructure );
    TIM_ITConfig ( TIM2, TIM_IT_Update | TIM_IT_Trigger, ENABLE );
    TIM_Cmd ( TIM2, ENABLE );
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init ( &NVIC_InitStructure );
}