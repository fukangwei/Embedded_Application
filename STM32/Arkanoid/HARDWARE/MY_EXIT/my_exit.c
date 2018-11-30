#include "stm32f10x.h"
#include "led.h"
#include "sys.h"
#include "my_exit.h"

volatile int move_left = 0; /* 向左移动标识符 */
volatile int move_right = 0; /* 向右移动标识符 */

void key_init ( void ) {
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_EXTILineConfig ( GPIO_PortSourceGPIOA, GPIO_PinSource0 );
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init ( &EXTI_InitStructure );
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init ( &NVIC_InitStructure );
    GPIO_EXTILineConfig ( GPIO_PortSourceGPIOA, GPIO_PinSource1 );
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init ( &EXTI_InitStructure );
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init ( &NVIC_InitStructure );
}

void EXTI0_IRQHandler ( void ) {
    if ( EXTI_GetITStatus ( EXTI_Line0 ) != RESET ) { /* 判断某个线上的中断是否发生 */
        /* 中断逻辑 */
        EXTI_ClearITPendingBit ( EXTI_Line0 ); /* 清除LINE上的中断标志位 */
        LED0 = !LED0;
        move_left = 1;
    }
}

void EXTI1_IRQHandler ( void ) {
    if ( EXTI_GetITStatus ( EXTI_Line1 ) != RESET ) { /* 判断某个线上的中断是否发生 */
        /* 中断逻辑 */
        EXTI_ClearITPendingBit ( EXTI_Line1 ); /* 清除LINE上的中断标志位 */
        LED1 = !LED1;
        move_right = 1;
    }
}