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

uint8 buffer[2048];/* ����һ��2KB�Ļ����� */

int main ( void ) {
    uint16 len = 0; /* ������ջ������ݵĳ��� */
    uint8 rIP[4]; /* ����ԴIP��ַ */
    uint16 rPort; /* ���ݵ�Դ�˿� */
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
    setRTR ( 2000 ); /* �������ʱ��ֵ */
    setRCR ( 3 ); /* ����������·��ʹ��� */

    while ( 1 ) {
        switch ( getSn_SR ( 0 ) ) {
            case SOCK_UDP:
                if ( getSn_IR ( 0 ) & Sn_IR_RECV ) {
                    setSn_IR ( 0, Sn_IR_RECV ); /* Sn_IR�ĵ�0λ��1 */
                }

                if ( ( len = getSn_RX_RSR ( 0 ) ) > 0 ) {
                    recvfrom ( 0, buffer, len, rIP, &rPort ); /* ��UDP�������ķ�ʽ�������� */
                    printf ( "%d.%d.%d.%d:%d\r\n", rIP[0], rIP[1], rIP[2], rIP[3], rPort ); /* �õ�UDP��ԴIP��ַ��Դ�˿� */
                    sendto ( 0, buffer, len, rIP, rPort ); /* W5500�ѽ��յ������ݷ��͸��Է� */
                    memset ( buffer, 0, sizeof ( buffer ) );
                }

            case SOCK_CLOSED:
                socket ( 0, Sn_MR_UDP, 3000, Sn_MR_CLOSE );
                break;
        }
    }
}

#define WIZ_RESET GPIO_Pin_3 /* RST������ΪPA3 */

void Reset_W5500 ( void ) { /* Ӳ����W5500 */
    GPIO_ResetBits ( GPIOA, WIZ_RESET );
    delay_us ( 2 );
    GPIO_SetBits ( GPIOA, WIZ_RESET );
    delay_ms ( 1600 );
}

void w5500_init ( void ) { /* w5500���ų�ʼ������ */
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA, ENABLE );
    GPIO_InitStructure.GPIO_Pin   = WIZ_RESET; /* ��ʼ��w5500��RST���ţ�PA3 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;  /* RSTģʽΪ��� */
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOA, WIZ_RESET );
    Reset_W5500();
    SPIx_Init(); /* ��ʼ��SPI�ڣ�����ʹ��SPI1 */
}