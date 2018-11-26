#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "TIM2.h"
#include "UltrasonicWave.h"

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    TIM2_Configuration();
    UltrasonicWave_Configuration();

    while ( 1 ) {
        UltrasonicWave_StartMeasure();
        delay_ms ( 1000 );
    }
}

