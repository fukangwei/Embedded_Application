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
            len = USART_RX_STA & 0x3f; /* 得到此次接收到的数据长度 */
            printf ( "\n您发送的消息为:\n" );

            for ( t = 0; t < len; t++ ) {
                USART3->DR = USART_RX_BUF[t];

                while ( ( USART3->SR & 0X40 ) == 0 ); /* 等待发送结束 */
            }

            printf ( "\n\n" ); /* 插入换行 */
            USART_RX_STA = 0;
        } else {
            times++;

            if ( times % 5000 == 0 ) {
                printf ( "\nMiniSTM32开发板 串口实验\n" );
                printf ( "正点原子@ALIENTEK\n\n\n" );
            }

            if ( times % 200 == 0 ) {
                printf ( "请输入数据,以回车键结束\n" );
            }

            if ( times % 30 == 0 ) {
                LED0 = !LED0;
            }

            delay_ms ( 10 );
        }
    }
}