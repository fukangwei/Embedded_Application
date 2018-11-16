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
    IWDG_Init ( 4, 625 ); /* 初始化看门狗，溢出时间为1s */
    printf ( "start!\r\n" );

    while ( 1 ) {
        printf ( "hello world\r\n" );
#if DEBUG
#else
        IWDG_Feed(); /* 喂狗操作 */
#endif
        delay_ms ( 500 );
    }
}