#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "adc.h"
#include "stdio.h"

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    Adc_Init(); /* ADC���ܳ�ʼ�� */

    while ( 1 ) {
        u16 adcx = 0;
        float temp = 0;
        adcx = Get_Adc ( ADC_Channel_0 ); /* ��ȡADCת���Ľ�� */
        printf ( "The ADC value is %d\r\n", adcx );
        temp = ( float ) adcx * ( 3.3 / 4096 );
        printf ( "the voltage is %.2f\r\n", temp ); /* ��ʾ��ѹֵ */
        delay_ms ( 500 );
    }
}