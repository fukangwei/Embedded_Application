#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "stmflash.h"
#include "string.h"

const u8 TEXT_Buffer[] = {"STM32 FLASH TEST"}; /* Ҫд�뵽STM32��FLASH���ַ������� */
#define SIZE sizeof(TEXT_Buffer) /* ���鳤�� */
#define FLASH_SAVE_ADDR  0X08020000 /* ����FLASH�ı����ַ(����Ϊż��������ֵҪ���ڱ�������ռ��FLASH�Ĵ�С + 0X08000000) */

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