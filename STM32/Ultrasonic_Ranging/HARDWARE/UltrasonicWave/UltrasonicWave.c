#include "UltrasonicWave.h"
#include "stdio.h"
#include "TIM2.h"
#include "delay.h"

#define TRIG_PORT GPIOC /* TRIG */
#define ECHO_PORT GPIOC /* ECHO */
#define TRIG_PIN  GPIO_Pin_8 /* TRIG */
#define ECHO_PIN  GPIO_Pin_9 /* ECHO */

unsigned int UltrasonicWave_Distance; /* ������ľ��� */

void UltrasonicWave_Configuration ( void ) { /* ������ģ��ĳ�ʼ�� */
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOC, ENABLE );
    GPIO_InitStructure.GPIO_Pin = TRIG_PIN; /* PC8��TRIG */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* ��Ϊ�������ģʽ */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( TRIG_PORT, &GPIO_InitStructure ); /* ��ʼ������GPIO */
    GPIO_InitStructure.GPIO_Pin = ECHO_PIN; /* PC9��ECH0 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; /* ��Ϊ���� */
    GPIO_Init ( ECHO_PORT, &GPIO_InitStructure ); /* ��ʼ��GPIOA */
}

void UltrasonicWave_CalculateTime ( void ) { /* ������� */
    UltrasonicWave_Distance = TIM_GetCounter ( TIM2 ) * 0.085;
}

void UltrasonicWave_StartMeasure ( void ) { /* ��ʼ��࣬����һ��>10us�����壬Ȼ��������صĸߵ�ƽʱ�� */
    GPIO_SetBits ( TRIG_PORT, TRIG_PIN ); /* �͡�> 10US���ĸߵ�ƽ */
    delay_us ( 20 ); /* ��ʱ20US */
    GPIO_ResetBits ( TRIG_PORT, TRIG_PIN );

    while ( !GPIO_ReadInputDataBit ( ECHO_PORT, ECHO_PIN ) ); /* �ȴ��ߵ�ƽ */

    TIM_SetCounter ( TIM2, 0 );
    TIM_Cmd ( TIM2, ENABLE ); /* ����ʱ�� */

    while ( GPIO_ReadInputDataBit ( ECHO_PORT, ECHO_PIN ) ); /* �ȴ��͵�ƽ */

    TIM_Cmd ( TIM2, DISABLE ); /* ��ʱ��2ʧ�� */
    UltrasonicWave_CalculateTime();
    TIM_SetCounter ( TIM2, 0 );
    printf ( "distance:%d cm\r\n", UltrasonicWave_Distance );
}