#include "udp_client.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/udp.h"
#include "usart.h"
#include "string.h"
#include "lwip_demo.h"

struct udp_pcb *udp_client_pcb;
struct pbuf *ubuf_client;

void udp_client_rev ( void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr , u16_t port ) {
    if ( p != NULL ) {
        if ( ( p->tot_len ) >= LWIP_DEMO_BUF ) {
            ( ( char * ) p->payload ) [199] = 0;
            memcpy ( lwip_demo_buf, p->payload, 200 );
        } else {
            memcpy ( lwip_demo_buf, p->payload, p->tot_len );
            lwip_demo_buf[p->tot_len] = 0;
        }

        lwip_flag |= LWIP_NEW_DATA;
        pbuf_free ( p );
    }
}

void udp_client_send_data ( void ) {
    err_t err;

    if ( ( lwip_flag & LWIP_SEND_DATA ) == LWIP_SEND_DATA ) {
        ubuf_client = pbuf_alloc ( PBUF_TRANSPORT, strlen ( ( char * ) lwip_demo_buf ), PBUF_RAM );
        ubuf_client->payload = lwip_demo_buf;
        err = udp_send ( udp_client_pcb, ubuf_client );

        if ( err != ERR_OK ) {
            lwip_log ( "UDP SERVER·¢ËÍÊý¾ÝÊ§°Ü£¡" );
        }

        lwip_flag &= ~LWIP_SEND_DATA;
        pbuf_free ( ubuf_client );
    }
}

#define UDP_CLIENT_PORT 1500


void Init_UDP_Client ( void ) {
    ip_addr_t ipaddr;
    IP4_ADDR ( &ipaddr, 192, 168, 1, 100 );
    udp_client_pcb = udp_new();

    if ( udp_client_pcb != NULL ) {
        udp_bind ( udp_client_pcb, IP_ADDR_ANY, UDP_CLIENT_PORT );
        udp_connect ( udp_client_pcb, &ipaddr, UDP_CLIENT_PORT );
        udp_recv ( udp_client_pcb, udp_client_rev, NULL );
    }
}