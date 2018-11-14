#include "pwm.h"
#include "led.h"

void PWM_Init ( u16 arr, u16 psc ) { /* PWM输出初始化，参数arr是自动重装值，psc是时钟预分频数 */
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    RCC_APB1PeriphClockCmd ( RCC_APB1Periph_TIM3, ENABLE );
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA  | RCC_APB2Periph_AFIO, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    TIM_TimeBaseStructure.TIM_Period = arr;
    TIM_TimeBaseStructure.TIM_Prescaler = psc;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit ( TIM3, &TIM_TimeBaseStructure );
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC2Init ( TIM3, &TIM_OCInitStructure );
    TIM_OC2PreloadConfig ( TIM3, TIM_OCPreload_Enable );
    TIM_ARRPreloadConfig ( TIM3, ENABLE );
    TIM_Cmd ( TIM3, ENABLE );
}