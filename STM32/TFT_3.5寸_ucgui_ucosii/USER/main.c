#include "led.h"
#include "sys.h"
#include "usart.h"
#include "ILI93xx.h"
#include "gui.h"
#include "ucos_ii.h"

#define DISABLE_JTAG do \
    { \
        RCC->APB2ENR |= 0x00000001; \
        AFIO->MAPR = ( 0x00FFFFFF & AFIO->MAPR ) | 0x04000000; \
    }while(0);

#define STARTUP_TASK_PRIO 6

#define TASK1_SIZE 80
#define TASK2_SIZE 1024

void SysTick_init ( void ) {
    SysTick_Config ( SystemCoreClock / OS_TICKS_PER_SEC );
}

void TestLed1 ( void *p_arg ) {
    while ( 1 ) {
        LED0 = !LED0;
        OSTimeDly ( 500 );
    }
}

void TestLed2 ( void *p_arg ) {
    GUI_Init();
    GUI_SetBkColor ( GUI_BLUE );
    GUI_SetColor ( GUI_RED );
    GUI_Clear();
    GUI_DrawCircle ( 100, 100, 50 );
    GUI_DispStringAt ( "Hello World ..", 10, 10 );

    while ( 1 ) {
        LED1 = !LED1;
        OSTimeDly ( 1000 );
    }
}

__align ( 8 ) static OS_STK task_testled1[TASK1_SIZE];
__align ( 8 ) static OS_STK task_testled2[TASK2_SIZE];

int main ( void ) {
    DISABLE_JTAG;
    SysTick_init();
    LED_Init();
    OSInit();
    OSTaskCreate ( TestLed1, ( void * ) 0, &task_testled1[TASK1_SIZE - 1], STARTUP_TASK_PRIO - 1 );
    OSTaskCreate ( TestLed2, ( void * ) 0, &task_testled2[TASK2_SIZE - 1], STARTUP_TASK_PRIO );
    OSStart();
    return 0;
}