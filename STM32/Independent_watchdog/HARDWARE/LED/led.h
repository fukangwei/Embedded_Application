#ifndef __LED_H
#define __LED_H	 
#include "stm32f10x.h"
//Mini STM32������
//LED��������			 
//����ԭ��@ALIENTEK
//2012/2/27

//LED�˿ڶ���
#define LED0 PBout(10)// PA8
#define LED1 PBout(11)// PD2	

void LED_Init(void);//��ʼ��

		 				    
#endif
