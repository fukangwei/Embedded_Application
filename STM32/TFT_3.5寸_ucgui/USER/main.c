#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "ILI93xx.h"
#include "gui.h"

#define DISABLE_JTAG do \
    { \
        RCC->APB2ENR |= 0x00000001; /* 开启afio时钟 */ \
        AFIO->MAPR = ( 0x00FFFFFF & AFIO->MAPR ) | 0x04000000; /* 关闭JTAG */ \
    }while(0);

int main ( void ) {
    DISABLE_JTAG; /* 关闭JTAG功能 */
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    GUI_Init();
    GUI_SetBkColor ( GUI_BLUE );
    GUI_SetColor ( GUI_RED );
    GUI_Clear();
    GUI_DrawCircle ( 100, 100, 50 );
    GUI_DispStringAt ( "Hello World ..", 10, 10 );

    while ( 1 ) {
    }
}