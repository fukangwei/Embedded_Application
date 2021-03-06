#include "tcp_client.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "usart.h"
#include "string.h"
#include "lwip_demo.h"

struct tcp_pcb *tcp_client_pcb; /* 定义一个TCP的协议控制块 */
static const char *respond =  "ALIENTEK STM32 Board Connected Successfully!\r\n";

enum tcp_client_states { /* 连接状态 */
    ES_NONE = 0,
    ES_RECEIVED, /* 接收到了数据 */
    ES_CLOSING /* 连接关闭 */
};

struct tcp_client_state { /* TCP服务器状态 */
    u8_t state;
};

err_t tcp_client_poll ( void *arg, struct tcp_pcb *tpcb ); /* 连接轮询时将要调用的函数 */
err_t tcp_client_connect ( void *arg, struct tcp_pcb *tpcb, err_t err ); /* 连接成功时将要调用的函数 */
void tcp_client_close ( struct tcp_pcb *tpcb, struct tcp_client_state *ts ); /* 关闭连接 */

err_t tcp_client_connect ( void *arg, struct tcp_pcb *tpcb, err_t err ) { /* 客户端成功连接到远程主机时调用 */
    struct tcp_client_state *ts;
    ts = arg;
    ts->state = ES_RECEIVED; /* 可以开始接收数据 */
    lwip_flag |= LWIP_CONNECTED; /* 标记连接成功 */
    tcp_write ( tpcb, respond, strlen ( respond ), 1 ); /* 回应信息 */
    return ERR_OK;
}

err_t tcp_client_poll ( void *arg, struct tcp_pcb *tpcb ) { /* 连接轮询时将要调用的函数 */
    err_t ret_err;
    struct tcp_client_state *ts;
    ts = arg;
    lwip_log ( "tcp_client_polling!\r\n" );

    if ( ts != NULL ) { /* 连接处于空闲可以发送数据 */
        if ( ( lwip_flag & LWIP_SEND_DATA ) == LWIP_SEND_DATA ) {
            tcp_write ( tpcb, lwip_demo_buf, strlen ( ( char * ) lwip_demo_buf ), 1 ); /* 发送数据 */
            lwip_flag &= ~LWIP_SEND_DATA; /* 清除发送数据的标志 */
        }
    } else {
        tcp_abort ( tpcb );
        ret_err = ERR_ABRT;
    }

    return ret_err;
}

void tcp_client_connect_remotehost ( void ) { /* 用于连接远程主机 */
    Init_TCP_Client();
}

err_t tcp_client_recv ( void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err ) { /* 客户端接收到数据之后将要调用的函数 */
    err_t ret_err;
    struct tcp_client_state *ts;
    ts = arg; /* TCP的PCB状态 */

    if ( p == NULL ) {
        ts->state = ES_CLOSING; /* 连接关闭 */
        tcp_client_close ( tpcb, ts );
        lwip_flag &= ~ LWIP_CONNECTED;  /* 清除连接标志 */
        printf ( "连接关闭了！1\r\n" );
    } else if ( err != ERR_OK ) {
        if ( p != NULL ) {
            pbuf_free ( p );
        }

        ret_err = err;
    } else if ( ts->state == ES_RECEIVED ) { /* 收到了新的数据 */
        if ( ( p->tot_len ) >= LWIP_DEMO_BUF ) { /* 如果收到的数据大于缓存 */
            ( ( char * ) p->payload ) [199] = 0;
            memcpy ( lwip_demo_buf, p->payload, 200 );
        } else {
            memcpy ( lwip_demo_buf, p->payload, p->tot_len );
            lwip_demo_buf[p->tot_len] = 0;
        }

        lwip_flag |= LWIP_NEW_DATA;
        tcp_recved ( tpcb, p->tot_len );
        pbuf_free ( p ); /* 释放内存 */
        ret_err = ERR_OK;
    } else if ( ts->state == ES_CLOSING ) { /* 服务器关闭了 */
        tcp_recved ( tpcb, p->tot_len );
        pbuf_free ( p );
        ret_err = ERR_OK;
        printf ( "连接关闭了！2\r\n" );
    } else { /* 其他未知状态 */
        tcp_recved ( tpcb, p->tot_len );
        pbuf_free ( p );
        ret_err = ERR_OK;
    }

    return ret_err;
}

void tcp_client_close ( struct tcp_pcb *tpcb, struct tcp_client_state *ts ) { /* 关闭连接 */
    tcp_arg ( tcp_client_pcb, NULL );
    tcp_recv ( tcp_client_pcb, NULL );
    tcp_poll ( tcp_client_pcb, NULL, 0 );

    if ( ts != NULL ) {
        mem_free ( ts );
    }

    tcp_close ( tpcb );
}

#define TCP_CLIENT_PORT 1300

void Init_TCP_Client ( void ) { /* 初始化TCP客户端 */
    struct tcp_client_state *ts;
    ip_addr_t ipaddr;
    IP4_ADDR ( &ipaddr, 192, 168, 1, 100 );
    tcp_client_pcb = tcp_new(); /* 新建一个PCB */

    if ( tcp_client_pcb != NULL ) {
        ts = mem_malloc ( sizeof ( struct tcp_client_state ) ); /* 申请内存 */
        tcp_arg ( tcp_client_pcb, ts );
        tcp_connect ( tcp_client_pcb, &ipaddr, TCP_CLIENT_PORT, tcp_client_connect );
        tcp_recv ( tcp_client_pcb, tcp_client_recv );
        tcp_poll ( tcp_client_pcb, tcp_client_poll, 0 ); /* 指定轮询时将要调用的回调函数 */
    }
}