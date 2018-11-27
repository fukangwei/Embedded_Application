#include "tcp_server.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "usart.h"
#include "string.h"
#include "lwip_demo.h"

struct tcp_pcb *tcp_server_pcb;
static const char *respond =  "ALIENTEK STM32 Board Connected Successfully!\r\n";

enum tcp_server_states {
    ES_NONE = 0,
    ES_RECEIVED,
    ES_CLOSING
};

struct tcp_server_state {
    u8_t state;
};

err_t tcp_server_accept ( void *arg, struct tcp_pcb *newpcb, err_t err );
err_t tcp_server_recv ( void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err );
err_t tcp_server_poll ( void *arg, struct tcp_pcb *tpcb );
void tcp_server_error ( void *arg, err_t err );
void tcp_server_close ( struct tcp_pcb *tpcb, struct tcp_server_state *ts );

#define TCP_SERVER_PORT 1200

void Init_TCP_Server ( void ) {
    err_t err;
    tcp_server_pcb = tcp_new();

    if ( tcp_server_pcb != NULL ) {
        err = tcp_bind ( tcp_server_pcb, IP_ADDR_ANY, TCP_SERVER_PORT );

        if ( err == ERR_OK ) {
            tcp_server_pcb = tcp_listen ( tcp_server_pcb );
            tcp_accept ( tcp_server_pcb, tcp_server_accept );
        }
    }
}

err_t tcp_server_accept ( void *arg, struct tcp_pcb *newpcb, err_t err ) {
    err_t ret_err;
    struct tcp_server_state *ts;
    ts = mem_malloc ( sizeof ( struct tcp_server_state ) );

    if ( ts != NULL ) {
        ts->state = ES_RECEIVED;
        lwip_flag |= LWIP_CONNECTED;
        tcp_write ( newpcb, respond, strlen ( respond ), 1 );
        tcp_arg ( newpcb, ts );
        tcp_recv ( newpcb, tcp_server_recv );
        tcp_err ( newpcb, tcp_server_error );
        tcp_poll ( newpcb, tcp_server_poll, 0 );
        ret_err = ERR_OK;
    } else {
        ret_err = ERR_MEM;
    }

    return ret_err;
}

err_t tcp_server_poll ( void *arg, struct tcp_pcb *tpcb ) {
    err_t ret_err;
    struct tcp_server_state *ts;
    ts = arg;
    lwip_log ( "tcp_server_polling!\r\n" );

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

err_t tcp_server_recv ( void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err ) {
    err_t ret_err;
    struct tcp_server_state *ts;
    ts = arg;

    if ( p == NULL ) {
        ts->state = ES_CLOSING;
        tcp_server_close ( tpcb, ts );
        lwip_flag &= ~ LWIP_CONNECTED;
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
    } else {
        tcp_recved ( tpcb, p->tot_len );
        pbuf_free ( p );
        ret_err = ERR_OK;
    }

    return ret_err;
}

void tcp_server_error ( void *arg, err_t err ) {
    struct tcp_server_state *ts;
    ts = arg;

    if ( ts != NULL ) {
        mem_free ( ts );
    }
}

void tcp_server_close ( struct tcp_pcb *tpcb, struct tcp_server_state *ts ) {
    if ( ts != NULL ) {
        mem_free ( ts );
    }

    tcp_close ( tpcb );
}