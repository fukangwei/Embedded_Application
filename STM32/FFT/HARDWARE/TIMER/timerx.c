#include "timerx.h"
#include "led.h"

volatile uint8_t ADC_TimeOutFlag = 0;

void TIM2_IRQHandler ( void ) {
    if ( TIM_GetITStatus ( TIM2, TIM_IT_Update ) != RESET ) {
        LED0 = !LED0;
        ADC_TimeOutFlag = 1;
    }

    TIM_ClearITPendingBit ( TIM2, TIM_IT_Update );
}

void TIM2_Int_Init ( u16 arr, u16 psc ) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
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