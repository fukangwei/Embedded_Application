#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "stmflash.h"
#include "string.h"

const u8 TEXT_Buffer[] = {"STM32 FLASH TEST"}; /* 要写入到STM32中FLASH的字符串数组 */
#define SIZE sizeof(TEXT_Buffer) /* 数组长度 */
#define FLASH_SAVE_ADDR  0X08020000 /* 设置FLASH的保存地址(必须为偶数，且其值要大于本代码所占用FLASH的大小 + 0X08000000) */

int main ( void ) {
    u8 datatemp[SIZE];
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    STMFLASH_Write ( FLASH_SAVE_ADDR, ( u16 * ) TEXT_Buffer, SIZE );
    printf ( "I write %s\r\n",  TEXT_Buffer ) ;
    delay_ms ( 500 );

    while ( 1 ) {
        memset ( datatemp, 0, sizeof ( datatemp ) );
        STMFLASH_Read ( FLASH_SAVE_ADDR, ( u16 * ) datatemp, SIZE );
        printf ( "I read %s\r\n",  datatemp ) ;
        LED = !LED;
        delay_ms ( 500 );
    }
}