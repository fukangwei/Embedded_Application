#include "led.h"
#include "sys.h"
#include "usart.h"
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "ucos_ii.h"

#define SystemFrequency       72000000
#define STARTUP_TASK_PRIO     8
#define STARTUP_TASK_STK_SIZE 80

void SysTick_init ( void ) {
    SysTick_Config ( SystemCoreClock / OS_TICKS_PER_SEC );
}

void TestLed1 ( void *p_arg ) {
    while ( 1 ) {
        LED0 = !LED0;
        OSTimeDlyHMSM ( 0, 0, 0, 500 );
    }
}

void TestLed2 ( void *p_arg ) {
    while ( 1 ) {
        printf ( "hello\r\n" );
        OSTimeDlyHMSM ( 0, 0, 2, 0 );
    }
}

static OS_STK task_testled1[STARTUP_TASK_STK_SIZE];
static OS_STK task_testled2[STARTUP_TASK_STK_SIZE];

int main ( void ) {
    SysTick_init();
    LED_Init();
    uart_init ( 9600 );
    OSInit();
    OSTaskCreate ( TestLed1, ( void * ) 0, &task_testled1[STARTUP_TASK_STK_SIZE - 1], STARTUP_TASK_PRIO - 1 );
    OSTaskCreate ( TestLed2, ( void * ) 0, &task_testled2[STARTUP_TASK_STK_SIZE - 1], STARTUP_TASK_PRIO );
    OSStart();
    return 0;
}