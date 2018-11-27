#include "tcp_client.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "usart.h"
#include "string.h"
#include "lwip_demo.h"

struct tcp_pcb *tcp_client_pcb;
static const char *respond =  "ALIENTEK STM32 Board Connected Successfully!\r\n";

enum tcp_client_states {
    ES_NONE = 0,
    ES_RECEIVED,
    ES_CLOSING
};

struct tcp_client_state {
    u8_t state;
};

err_t tcp_client_poll ( void *arg, struct tcp_pcb *tpcb );
err_t tcp_client_connect ( void *arg, struct tcp_pcb *tpcb, err_t err );
void tcp_client_close ( struct tcp_pcb *tpcb, struct tcp_client_state *ts );

err_t tcp_client_connect ( void *arg, struct tcp_pcb *tpcb, err_t err ) {
    struct tcp_client_state *ts;
    ts = arg;
    ts->state = ES_RECEIVED;
    lwip_flag |= LWIP_CONNECTED;
    tcp_write ( tpcb, respond, strlen ( respond ), 1 );
    return ERR_OK;
}

err_t tcp_client_poll ( void *arg, struct tcp_pcb *tpcb ) {
    err_t ret_err;
    struct tcp_client_state *ts;
    ts = arg;
    lwip_log ( "tcp_client_polling!\r\n" );

    if ( ts != NULL ) {
        if ( ( lwip_flag & LWIP_SEND_DATA ) == LWIP_SEND_DATA ) {
            tcp_write ( tpcb, lwip_demo_buf, strlen ( ( char * ) lwip_demo_buf ), 1 );
            lwip_flag &= ~LWIP_SEND_DATA;
        }
    } else {
        tcp_abort ( tpcb );
        ret_err = ERR_ABRT;
    }

    return ret_err;
}

void tcp_client_connect_remotehost ( void ) {
    Init_TCP_Client();
}

err_t tcp_client_recv ( void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err ) {
    err_t ret_err;
    struct tcp_client_state *ts;
    ts = arg;

    if ( p == NULL ) {
        ts->state = ES_CLOSING;
        tcp_client_close ( tpcb, ts );
        lwip_flag &= ~ LWIP_CONNECTED;
        printf ( "连接关闭了！1\r\n" );
    } else if ( err != ERR_OK ) {
        if ( p != NULL ) {
            pbuf_free ( p );
        }

        ret_err = err;
    } else if ( ts->state == ES_RECEIVED ) {
        if ( ( p->tot_len ) >= LWIP_DEMO_BUF ) {
            ( ( char * ) p->payload ) [199] = 0;
            memcpy ( lwip_demo_buf, p->payload, 200 );
        } else {
            memcpy ( lwip_demo_buf, p->payload, p->tot_len );
            lwip_demo_buf[p->tot_len] = 0;
        }

        lwip_flag |= LWIP_NEW_DATA;
        tcp_recved ( tpcb, p->tot_len );
        pbuf_free ( p );
        ret_err = ERR_OK;
    } else if ( ts->state == ES_CLOSING ) {
        tcp_recved ( tpcb, p->tot_len );
        pbuf_free ( p );
        ret_err = ERR_OK;
        printf ( "连接关闭了！2\r\n" );
    } else {
        tcp_recved ( tpcb, p->tot_len );
        pbuf_free ( p );
        ret_err = ERR_OK;
    }

    return ret_err;
}

void tcp_client_close ( struct tcp_pcb *tpcb, struct tcp_client_state *ts ) {
    tcp_arg ( tcp_client_pcb, NULL );
    tcp_recv ( tcp_client_pcb, NULL );
    tcp_poll ( tcp_client_pcb, NULL, 0 );

    if ( ts != NULL ) {
        mem_free ( ts );
    }

    tcp_close ( tpcb );
}

#define TCP_CLIENT_PORT 1300

void Init_TCP_Client ( void ) {
    struct tcp_client_state *ts;
    ip_addr_t ipaddr;
    IP4_ADDR ( &ipaddr, 192, 168, 1, 100 );
    tcp_client_pcb = tcp_new();

    if ( tcp_client_pcb != NULL ) {
        ts = mem_malloc ( sizeof ( struct tcp_client_state ) );
        tcp_arg ( tcp_client_pcb, ts );
        tcp_connect ( tcp_client_pcb, &ipaddr, TCP_CLIENT_PORT, tcp_client_connect );
        tcp_recv ( tcp_client_pcb, tcp_client_recv );
        tcp_poll ( tcp_client_pcb, tcp_client_poll, 0 );
    }
}