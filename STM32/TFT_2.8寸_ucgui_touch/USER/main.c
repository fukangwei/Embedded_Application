#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "GUI.h"
#include "touch.h"

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    GUI_Init();
    Touch_Init();
    GUI_SetBkColor ( GUI_RED );
    GUI_SetColor ( GUI_WHITE );
    GUI_Clear();
    GUI_DispStringAt ( "Hello World ..", 10, 10 );
    GUI_CURSOR_Show(); /* 显示鼠标，测试触摸屏 */

    while ( 1 ) {
        GUI_TOUCH_Exec(); /* 调用UCGUI TOUCH相关函数 */
        GUI_Exec(); /* GUI事件更新 */
    }
}