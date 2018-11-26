#include "UltrasonicWave.h"
#include "stdio.h"
#include "TIM2.h"
#include "delay.h"

#define TRIG_PORT GPIOC /* TRIG */
#define ECHO_PORT GPIOC /* ECHO */
#define TRIG_PIN  GPIO_Pin_8 /* TRIG */
#define ECHO_PIN  GPIO_Pin_9 /* ECHO */

unsigned int UltrasonicWave_Distance; /* 计算出的距离 */

void UltrasonicWave_Configuration ( void ) { /* 超声波模块的初始化 */
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOC, ENABLE );
    GPIO_InitStructure.GPIO_Pin = TRIG_PIN; /* PC8接TRIG */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* 设为推挽输出模式 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( TRIG_PORT, &GPIO_InitStructure ); /* 初始化外设GPIO */
    GPIO_InitStructure.GPIO_Pin = ECHO_PIN; /* PC9接ECH0 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; /* 设为输入 */
    GPIO_Init ( ECHO_PORT, &GPIO_InitStructure ); /* 初始化GPIOA */
}

void UltrasonicWave_CalculateTime ( void ) { /* 计算距离 */
    UltrasonicWave_Distance = TIM_GetCounter ( TIM2 ) * 0.085;
}

void UltrasonicWave_StartMeasure ( void ) { /* 开始测距，发送一个>10us的脉冲，然后测量返回的高电平时间 */
    GPIO_SetBits ( TRIG_PORT, TRIG_PIN ); /* 送“> 10US”的高电平 */
    delay_us ( 20 ); /* 延时20US */
    GPIO_ResetBits ( TRIG_PORT, TRIG_PIN );

    while ( !GPIO_ReadInputDataBit ( ECHO_PORT, ECHO_PIN ) ); /* 等待高电平 */

    TIM_SetCounter ( TIM2, 0 );
    TIM_Cmd ( TIM2, ENABLE ); /* 开启时钟 */

    while ( GPIO_ReadInputDataBit ( ECHO_PORT, ECHO_PIN ) ); /* 等待低电平 */

    TIM_Cmd ( TIM2, DISABLE ); /* 定时器2失能 */
    UltrasonicWave_CalculateTime();
    TIM_SetCounter ( TIM2, 0 );
    printf ( "distance:%d cm\r\n", UltrasonicWave_Distance );
}