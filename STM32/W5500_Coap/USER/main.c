#include "led.h"
#include "spi.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "socket.h"
#include "device.h"
#include "w5500.h"
#include <stdbool.h>
#include "string.h"
#include "coap.h"

void Reset_W5500 ( void );
void w5500_init ( void );

extern uint8 txsize[];
extern uint8 rxsize[];

#define PORT 5683
#define SIZE 128
uint8_t buf[SIZE];
uint8_t scratch_raw[SIZE];
int n, rc;
uint16 len = 0; /* 定义接收或发送数据的长度 */
coap_packet_t pkt;

int main ( void ) {
    uint8 rIP[4]; /* 数据源IP地址 */
    uint16 rPort; /* 数据的源端口 */
    coap_rw_buffer_t scratch_buf = {scratch_raw, sizeof ( scratch_raw ) };
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
    setRTR ( 2000 ); /* 设置溢出时间值 */
    setRCR ( 3 ); /* 设置最大重新发送次数 */
    endpoint_setup();

    while ( 1 ) {
        switch ( getSn_SR ( 0 ) ) {
            case SOCK_UDP:
                if ( getSn_IR ( 0 ) & Sn_IR_RECV ) {
                    setSn_IR ( 0, Sn_IR_RECV ); /* Sn_IR的第0位置1 */
                }

                if ( ( len = getSn_RX_RSR ( 0 ) ) > 0 ) {
                    /*---------------------------------- Coap PArt -------------------------------------*/
                    n = recvfrom ( 0, buf, len, rIP, &rPort ); /* 以UDP服务器的方式接收数据 */
#ifdef DEBUG
                    printf ( "%d.%d.%d.%d:%d\r\n", rIP[0], rIP[1], rIP[2], rIP[3], rPort ); /* 得到UDP的源IP地址和源端口 */
                    printf ( "Received: " );
                    coap_dump ( buf, n, true );
                    printf ( "\r\n" );
#endif

                    if ( 0 != ( rc = coap_parse ( &pkt, buf, n ) ) ) {
                        printf ( "Bad packet rc = %d\r\n", rc );
                    } else {
                        size_t rsplen = sizeof ( buf );
                        coap_packet_t rsppkt;
#ifdef DEBUG
                        coap_dumpPacket ( &pkt );
#endif
                        coap_handle_req ( &scratch_buf, &pkt, &rsppkt );

                        if ( 0 != ( rc = coap_build ( buf, &rsplen, &rsppkt ) ) ) {
                            printf ( "coap_build failed rc = %d\r\n", rc );
                        } else {
#ifdef DEBUG
                            printf ( "Sending: " );
                            coap_dump ( buf, rsplen, true );
                            printf ( "\r\n" );
#endif
#ifdef DEBUG
                            coap_dumpPacket ( &rsppkt );
#endif
                            sendto ( 0, buf, rsplen, rIP, rPort ); /* W5500把接收到的数据发送给对方 */
                        }
                    }
                }

                LED0 = !LED0;

            case SOCK_CLOSED:
                socket ( 0, Sn_MR_UDP, PORT, Sn_MR_CLOSE );
                break;
        }
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