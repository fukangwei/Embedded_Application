#include "led.h"
#include "spi.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "socket.h"
#include "device.h"
#include "w5500.h"
#include "util.h"
#include "httputil.h"

void Reset_W5500 ( void );
void w5500_init ( void );

int main ( void ) {
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    w5500_init();
    printf ( "W5500 initialized!\r\n" );
    set_default();
    set_network();
    printf ( "Network is ready.\r\n" );

    while ( 1 ) {
        do_http();
    }
}

#define WIZ_RESET GPIO_Pin_3

void Reset_W5500 ( void ) {
    GPIO_ResetBits ( GPIOA, WIZ_RESET );
    delay_us ( 2 );
    GPIO_SetBits ( GPIOA, WIZ_RESET );
    delay_ms ( 1600 );
}

void w5500_init ( void ) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA, ENABLE );
    GPIO_InitStructure.GPIO_Pin   = WIZ_RESET;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOA, WIZ_RESET );
    Reset_W5500();
    SPIx_Init();
}