#include "TIM2.h"

void TIM2_Configuration ( void ) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd ( RCC_APB1Periph_TIM2 , ENABLE );
    TIM_DeInit ( TIM2 );
    TIM_TimeBaseStructure.TIM_Period = 65535; /* �Զ���װ�ؼĴ������ڵ�ֵ(����ֵ) */
    /* �ۼ�TIM_Period��Ƶ�ʺ����һ�����»����ж� */
    TIM_TimeBaseStructure.TIM_Prescaler = ( 360 - 1 ); /* ʱ��Ԥ��Ƶ����72M/360�� */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; /* ������Ƶ */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; /* ���ϼ���ģʽ */
    TIM_TimeBaseInit ( TIM2, &TIM_TimeBaseStructure );
    TIM_ClearFlag ( TIM2, TIM_FLAG_Update ); /* �������жϱ�־ */
}