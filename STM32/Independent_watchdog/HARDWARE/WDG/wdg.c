#include "wdg.h"
#include "led.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_wwdg.h"

void IWDG_Init ( u8 prer, u16 rlr ) {
    IWDG_WriteAccessCmd ( IWDG_WriteAccess_Enable ); /* ʹ�ܶԼĴ���IWDG_PR��IWDG_RLR��д���� */
    IWDG_SetPrescaler ( prer ); /* ����IWDGԤ��ƵֵΪ64 */
    IWDG_SetReload ( rlr ); /* ����IWDG��װ��ֵ */
    IWDG_ReloadCounter(); /* ����IWDG��װ�ؼĴ�����ֵ��װ��IWDG������ */
    IWDG_Enable(); /* ʹ��IWDG */
}

void IWDG_Feed ( void ) { /* ι�������Ź� */
    IWDG_ReloadCounter();
}

void WWDG_Init ( u8 tr, u8 wr, u32 fprer ) {
    RCC_APB1PeriphClockCmd ( RCC_APB1Periph_WWDG, ENABLE ); /* WWDGʱ��ʹ�� */
    WWDG_SetPrescaler ( fprer ); /* ����IWDGԤ��Ƶֵ */
    WWDG_SetWindowValue ( wr ); /* ���ô���ֵ */
    WWDG_Enable ( tr ); /* ʹ�ܿ��Ź� */
    WWDG_ClearFlag();
    WWDG_NVIC_Init(); /* ��ʼ�����ڿ��Ź�NVIC */
    WWDG_EnableIT(); /* �������ڿ��Ź��ж� */
}

void WWDG_Set_Counter ( u8 cnt ) { /* ������WWDG��������ֵ */
    WWDG_Enable ( cnt );
}

void WWDG_NVIC_Init() { /* ���ڿ��Ź��жϷ������ */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn; /* WWDG�ж� */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_Init ( &NVIC_InitStructure ); /* NVIC��ʼ�� */
}

void WWDG_IRQHandler ( void ) {
    WWDG_SetCounter ( 0x7F ); /* ��������˾䣬���ڿ��Ź���������λ */
    WWDG_ClearFlag(); /* �����ǰ�����жϱ�־λ */
}