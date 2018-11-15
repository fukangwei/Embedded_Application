#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "stdio.h"
#include "tsensor.h"

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    T_Adc_Init(); /* STM32�ڲ�ת����ʼ�� */

    while ( 1 ) {
        u16 adcx = 0;
        float temp = 0.0f;
        float temperate = 0.0f;
        adcx = T_Get_Temp(); /* ��ȡADCת���Ľ�� */
        printf ( "The ADC value is %d\r\n", adcx );
        temp = ( float ) adcx * ( 3.3 / 4096 );
        printf ( "The voltage is %.2f\r\n", temp ); /* ��ʾ��ѹ��ֵ */
        temperate = temp; /* �����¶ȴ������ĵ�ѹֵ */
        temperate = ( 1.43 - temperate ) / 0.0043 + 25; /* �������ǰ�¶�ֵ */
        printf ( "The temperate is %.2f\r\n", temperate );
        delay_ms ( 500 );
    }
}