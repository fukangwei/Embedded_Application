#include "delay.h"
#include "led.h"
#include "sys.h"
#include "uip.h"
#include "uip_arp.h"
#include "usart.h"
#include "tapdev.h"
#include "timer.h"
#include "enc28j60.h"

void uip_polling ( void );
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

int main ( void ) {
    uip_ipaddr_t ipaddr;
    delay_init();
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    tapdev_init();
    uip_init();
    uip_ipaddr ( ipaddr, 192, 168, 1, 230 );
    uip_sethostaddr ( ipaddr );
    uip_ipaddr ( ipaddr, 192, 168, 1, 1 );
    uip_setdraddr ( ipaddr );
    uip_ipaddr ( ipaddr, 255, 255, 255, 0 );
    uip_setnetmask ( ipaddr );

    while ( 1 ) {
        uip_polling();
        delay_ms ( 2 );
        printf ( "OK!\n" );
    }
}

void uip_polling ( void ) {
    u8 i;
    static struct timer periodic_timer, arp_timer;
    static u8 timer_ok = 0;

    if ( timer_ok == 0 ) {
        timer_ok = 1;
        timer_set ( &periodic_timer, CLOCK_SECOND / 2 );
        timer_set ( &arp_timer, CLOCK_SECOND * 10 );
    }

    uip_len = tapdev_read();

    if ( uip_len > 0 ) {
        if ( BUF->type == htons ( UIP_ETHTYPE_IP ) ) {
            uip_arp_ipin();
            uip_input();

            if ( uip_len > 0 ) {
                uip_arp_out();
                tapdev_send();
            }
        } else if ( BUF->type == htons ( UIP_ETHTYPE_ARP ) ) {
            uip_arp_arpin();

            if ( uip_len > 0 ) {
                tapdev_send();
            }
        }
    } else if ( timer_expired ( &periodic_timer ) ) {
        timer_reset ( &periodic_timer );

        for ( i = 0; i < UIP_CONNS; i++ ) {
            uip_periodic ( i );

            if ( uip_len > 0 ) {
                uip_arp_out();
                tapdev_send();
            }
        }

#if UIP_UDP

        for ( i = 0; i < UIP_UDP_CONNS; i++ ) {
            uip_udp_periodic ( i );

            if ( uip_len > 0 ) {
                uip_arp_out();
                tapdev_send();
            }
        }

#endif

        if ( timer_expired ( &arp_timer ) ) {
            timer_reset ( &arp_timer );
            uip_arp_timer();
        }
    }
}