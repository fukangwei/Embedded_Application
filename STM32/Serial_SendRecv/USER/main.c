#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"

int main ( void ) {
    u8 t;
    u8 len;
    u16 times = 0;
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();

    while ( 1 ) {
        if ( USART_RX_STA & 0x80 ) {
            len = USART_RX_STA & 0x3f; /* �õ��˴ν��յ������ݳ��� */
            printf ( "\n�����͵���ϢΪ:\n" );

            for ( t = 0; t < len; t++ ) {
                USART3->DR = USART_RX_BUF[t];

                while ( ( USART3->SR & 0X40 ) == 0 ); /* �ȴ����ͽ��� */
            }

            printf ( "\n\n" ); /* ���뻻�� */
            USART_RX_STA = 0;
        } else {
            times++;

            if ( times % 5000 == 0 ) {
                printf ( "\nMiniSTM32������ ����ʵ��\n" );
                printf ( "����ԭ��@ALIENTEK\n\n\n" );
            }

            if ( times % 200 == 0 ) {
                printf ( "����������,�Իس�������\n" );
            }

            if ( times % 30 == 0 ) {
                LED0 = !LED0;
            }

            delay_ms ( 10 );
        }
    }
}