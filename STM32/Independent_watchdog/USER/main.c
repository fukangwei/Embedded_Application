#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "wdg.h"

#define DEBUG 1

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    IWDG_Init ( 4, 625 ); /* ��ʼ�����Ź������ʱ��Ϊ1s */
    printf ( "start!\r\n" );

    while ( 1 ) {
        printf ( "hello world\r\n" );
#if DEBUG
#else
        IWDG_Feed(); /* ι������ */
#endif
        delay_ms ( 500 );
    }
}