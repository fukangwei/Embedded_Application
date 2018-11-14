#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "pwm.h"

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    PWM_Init ( 900, 0 ); /* ����Ƶ��PWMƵ�� = 72000000/900 = 80Khz */

    while ( 1 ) {
        TIM_SetCompare2 ( TIM3, 100 ); /* ռ�ձ�Ϊ(900 - 100)/100 */
    }
}