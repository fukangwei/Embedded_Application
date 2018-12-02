#include "delay.h"
#include "led.h"
#include "sys.h"
#include "uip.h"
#include "uip_arp.h"
#include "usart.h"
#include "tapdev.h"
#include "timer.h"
#include "enc28j60.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

void uip_polling ( void );

int main ( void ) {
    uip_ipaddr_t ipaddr;
    delay_init();
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    tapdev_init();
    uip_init(); /* uIP��ʼ�� */
    uip_ipaddr ( ipaddr, 192, 168, 1, 230 ); /* ���ñ�������IP��ַ */
    uip_sethostaddr ( ipaddr );
    uip_ipaddr ( ipaddr, 192, 168, 1, 1 ); /* ��������IP��ַ(·����IP��ַ) */
    uip_setdraddr ( ipaddr );
    uip_ipaddr ( ipaddr, 255, 255, 255, 0 ); /* ������������ */
    uip_setnetmask ( ipaddr );
    hello_world_init();

    while ( 1 ) {
        uip_polling(); /* ����uip�¼���������뵽�û������ѭ������ */
    }
}

void uip_polling ( void ) {
    u8 i;
    static struct timer periodic_timer, arp_timer;
    static u8 timer_ok = 0;

    if ( timer_ok == 0 ) { /* ����ʼ��һ�� */
        timer_ok = 1;
        timer_set ( &periodic_timer, CLOCK_SECOND / 2 ); /* ����1��0.5��Ķ�ʱ�� */
        timer_set ( &arp_timer, CLOCK_SECOND * 10 ); /* ����1��10��Ķ�ʱ�� */
    }

    uip_len = tapdev_read(); /* �������豸��ȡһ��IP�����õ����ݳ��ȣ�uip_len��uip.c�ж��� */

    if ( uip_len > 0 ) { /* ������ */
        /* ����IP���ݰ�(ֻ��У��ͨ����IP���Żᱻ����) */
        if ( BUF->type == htons ( UIP_ETHTYPE_IP ) ) { /*�Ƿ�ΪIP�� */
            uip_arp_ipin(); /* ȥ����̫��ͷ�ṹ������ARP�� */
            uip_input(); /* IP������ */

            if ( uip_len > 0 ) { /* �����Ҫ�������ݣ������ݴ洢��uip_buf��������uip_len(����2��ȫ�ֱ���) */
                uip_arp_out(); /* ������̫��ͷ�ṹ������������ʱ����Ҫ����ARP���� */
                tapdev_send(); /* �������ݵ���̫�� */
            }
        } else if ( BUF->type == htons ( UIP_ETHTYPE_ARP ) ) { /* �Ƿ���ARP����� */
            uip_arp_arpin();

            if ( uip_len > 0 ) { /* �����Ҫ�������ݣ������ݴ洢��uip_buf��������uip_len */
                tapdev_send(); /* �����Ҫ�������ݣ���ͨ��tapdev_send���� */
            }
        }
    } else if ( timer_expired ( &periodic_timer ) ) { /* 0.5�붨ʱ����ʱ */
        timer_reset ( &periodic_timer ); /* ��λ0.5�붨ʱ�� */

        for ( i = 0; i < UIP_CONNS; i++ ) { /* ��������ÿ��TCP���ӣ�UIP_CONNSȱʡ��40�� */
            uip_periodic ( i ); /* ����TCPͨ���¼� */

            if ( uip_len > 0 ) { /* �����Ҫ�������ݣ������ݴ洢��uip_buf��������uip_len(����2��ȫ�ֱ���) */
                uip_arp_out(); /* ������̫��ͷ�ṹ������������ʱ����Ҫ����ARP���� */
                tapdev_send(); /* �������ݵ���̫�� */
            }
        }

#if UIP_UDP

        for ( i = 0; i < UIP_UDP_CONNS; i++ ) { /* ��������ÿ��UDP���ӣ�UIP_UDP_CONNSȱʡ��10�� */
            uip_udp_periodic ( i ); /* ����UDPͨ���¼� */

            if ( uip_len > 0 ) { /* �����Ҫ�������ݣ������ݴ洢��uip_buf��������uip_len(����2��ȫ�ֱ���) */
                uip_arp_out(); /* ������̫��ͷ�ṹ������������ʱ����Ҫ����ARP���� */
                tapdev_send(); /* �������ݵ���̫�� */
            }
        }

#endif

        if ( timer_expired ( &arp_timer ) ) { /* ÿ��10�����1��ARP��ʱ�����������ڶ���ARP���� */
            timer_reset ( &arp_timer );
            uip_arp_timer();
        }
    }
}