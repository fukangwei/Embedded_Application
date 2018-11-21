#include "led.h"
#include "spi.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "socket.h"
#include "device.h"
#include "w5500.h"
#include "string.h"

void Reset_W5500 ( void );
void w5500_init ( void );

extern uint8 txsize[];
extern uint8 rxsize[];

uint8 buffer[2048];/* 定义一个2KB的缓存区 */

int main ( void ) {
    uint16 len = 0; /* 定义接收或发送数据的长度 */
    uint8 rIP[4]; /* 数据源IP地址 */
    uint16 rPort; /* 数据的源端口 */
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
    sysinit ( txsize, rxsize );
    delay_ms ( 1600 );
    setRTR ( 2000 ); /* 设置溢出时间值 */
    setRCR ( 3 ); /* 设置最大重新发送次数 */

    while ( 1 ) {
        switch ( getSn_SR ( 0 ) ) {
            case SOCK_UDP:
                if ( getSn_IR ( 0 ) & Sn_IR_RECV ) {
                    setSn_IR ( 0, Sn_IR_RECV ); /* Sn_IR的第0位置1 */
                }

                if ( ( len = getSn_RX_RSR ( 0 ) ) > 0 ) {
                    recvfrom ( 0, buffer, len, rIP, &rPort ); /* 以UDP服务器的方式接收数据 */
                    printf ( "%d.%d.%d.%d:%d\r\n", rIP[0], rIP[1], rIP[2], rIP[3], rPort ); /* 得到UDP的源IP地址和源端口 */
                    sendto ( 0, buffer, len, rIP, rPort ); /* W5500把接收到的数据发送给对方 */
                    memset ( buffer, 0, sizeof ( buffer ) );
                }

            case SOCK_CLOSED:
                socket ( 0, Sn_MR_UDP, 3000, Sn_MR_CLOSE );
                break;
        }
    }
}

#define WIZ_RESET GPIO_Pin_3 /* RST的引脚为PA3 */

void Reset_W5500 ( void ) { /* 硬重启W5500 */
    GPIO_ResetBits ( GPIOA, WIZ_RESET );
    delay_us ( 2 );
    GPIO_SetBits ( GPIOA, WIZ_RESET );
    delay_ms ( 1600 );
}

void w5500_init ( void ) { /* w5500引脚初始化函数 */
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA, ENABLE );
    GPIO_InitStructure.GPIO_Pin   = WIZ_RESET; /* 初始化w5500的RST引脚：PA3 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;  /* RST模式为输出 */
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOA, WIZ_RESET );
    Reset_W5500();
    SPIx_Init(); /* 初始化SPI口，这里使用SPI1 */
}