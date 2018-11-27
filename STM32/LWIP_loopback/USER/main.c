#include "delay.h"
#include "led.h"
#include "mysys.h"
#include "usart.h"
#include "enc28j60.h"
#include "lwip/init.h"
#include "lwip/ip.h"
#include "lwip/dhcp.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/dns.h"
#include "netif/etharp.h"
#include "netif/ethernetif.h"
#include "arch/sys_arch.h"

#define CLOCKTICKS_PER_MS 10 /* 定义时钟节拍 */

static ip_addr_t ipaddr, netmask, gw; /* 定义IP地址 */
struct netif enc28j60_netif; /* 定义网络接口 */
u32_t last_arp_time;
u32_t last_tcp_time;
u32_t last_ipreass_time;
u32_t input_time;

#define TCP_SERVER 1
#define LOOP_TEST  1

#if TCP_SERVER /* TCP服务器 */

static err_t tcpserver_recv ( void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err ) {
    if ( p != NULL ) { /* 当数据不为空时 */
        tcp_recved ( pcb, p->tot_len ); /* 通知内核更新接收窗口 */
        tcp_write ( pcb, p->payload, p->len, 1 ); /* 简单起见，只将第一个pbuf的数据返回给客户端 */
        pbuf_free ( p ); /* 释放整个数据包 */
    } else if ( err == ERR_OK ) { /* 如果发生接收错误 */
        return tcp_close ( pcb ); /* 关闭该连接 */
    }

    return ERR_OK; /* 返回操作结果 */
}

/* 当有客户端向服务器发起连接时，该函数会被系统回调执行 */
static err_t tcpserver_acccept ( void *arg, struct tcp_pcb *pcb, err_t err ) {
    tcp_recv ( pcb, tcpserver_recv ); /* 向该连接注册函数tcpserver_recv */
    return ERR_OK;
}

void tcpserver_init ( void ) {
    struct tcp_pcb *pcb;
    pcb = tcp_new(); /* 申请一个TCP控制块 */
    tcp_bind ( pcb, IP_ADDR_ANY, 8080 ); /* 将该控制块和本地端口8080绑定 */
    pcb = tcp_listen ( pcb ); /* 控制块为侦听状态，等待客户端的连接 */
    tcp_accept ( pcb, tcpserver_acccept ); /* 注册连接回调函数tcpserver_accept，当有客户端连接服务器时，该函数被系统回调 */
}

#endif

#if LOOP_TEST /* 环回接口测试代码 */

struct netif loop_netif;  /* 定义一个全局的环回接口管理结构 */

err_t loopif_init ( struct netif *netif ) {
    netif->name[0] = 'l';  /* 填写接口的名字 */
    netif->name[1] = 'o';
    netif->output  =  netif_loop_output;  /* 注册数据包输出函数 */
    return ERR_OK;
}

void loop_test_init ( void ) { /* 初始化环回接口 */
    IP4_ADDR ( &ipaddr, 127, 0, 0, 1 ); /* IP地址 */
    IP4_ADDR ( &gw, 127, 0, 0, 1 ); /* 网关地址 */
    IP4_ADDR ( &netmask, 255, 255, 255, 0 ); /* 子网掩码 */
    netif_add ( &loop_netif, &ipaddr, &netmask, &gw, NULL, loopif_init, NULL ); /* 调用netif_add注册环回接口 */
    netif_set_up ( &loop_netif ); /* 软件使能，设置flags字段 */
}

static err_t loopclient_recv ( void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err ) {
    if ( p != NULL ) { /* 若数据包不为空 */
        char *data = ( char * ) p->payload; /* 得到第一个pbuf中的数据起始地址 */
        int length = p->len;  /* 得到第一个pbuf中的数据长度  */
        tcp_recved ( pcb, p->tot_len ); /* 通告协议栈数据的接收 */
        printf ( "loopClient Get Data: " ); /* 打印信息 */

        while ( length-- > 0 ) {
            printf ( "%c", ( int ) ( *data++ ) ); /* 输出收到的每个字符 */
        }

        delay_ms ( 500 );
        tcp_write ( pcb, p->payload, p->len, 1 ); /* 再向服务器发送数据 */
        pbuf_free ( p );
    } else if ( err == ERR_OK ) {
        return tcp_close ( pcb );
    }

    return ERR_OK;
}

unsigned char loopdata[] = "Loop Interface Test!\n"; /* 发送给服务器的字符串 */

/* 客户端连接建立回调函数，当与服务器建立起TCP连接后，该函数会被调用 */
static err_t loopclient_connect ( void *arg, struct tcp_pcb *tpcb, err_t err ) {
    tcp_recv ( tpcb, loopclient_recv ); /* 注册数据接收回调函数 */
    tcp_write ( tpcb, loopdata, sizeof ( loopdata ) - 1, 1 ); /* 向服务器发送字符串数据 */
    return ERR_OK;
}

void loopclient_init ( void ) { /* 客户端初始化程序 */
    struct tcp_pcb *pcb;
    struct ip_addr ipaddr;
    IP4_ADDR ( &ipaddr, 127, 0, 0, 1 ); /* 服务器的目的IP地址，这里为本地地址 */
    pcb = tcp_new();  /* 申请一个TCP控制块 */
    tcp_bind ( pcb, IP_ADDR_ANY, 7 ); /* 为客户端程序绑定一个本地端口 */
    tcp_connect ( pcb, &ipaddr, 8080, loopclient_connect ); /* 与服务器的8080端口建立连接 */
}

#endif

void LWIP_Polling ( void ) {
    if ( timer_expired ( &input_time, 5 ) ) {
        ethernetif_input ( &enc28j60_netif );
    }

    if ( timer_expired ( &last_tcp_time, TCP_TMR_INTERVAL / CLOCKTICKS_PER_MS ) ) {
        tcp_tmr();
    }

    if ( timer_expired ( &last_arp_time, ARP_TMR_INTERVAL / CLOCKTICKS_PER_MS ) ) {
        etharp_tmr();
    }

    if ( timer_expired ( &last_ipreass_time, IP_TMR_INTERVAL / CLOCKTICKS_PER_MS ) ) {
        ip_reass_tmr();
    }

#if LWIP_DHCP>0

    if ( timer_expired ( &last_dhcp_fine_time, DHCP_FINE_TIMER_MSECS / CLOCKTICKS_PER_MS ) ) {
        dhcp_fine_tmr();
    }

    if ( timer_expired ( &last_dhcp_coarse_time, DHCP_COARSE_TIMER_MSECS / CLOCKTICKS_PER_MS ) ) {
        dhcp_coarse_tmr();
    }

#endif
}

int main ( void ) {
    delay_init();
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    IP4_ADDR ( &ipaddr, 192, 168, 1, 230 );
    IP4_ADDR ( &gw, 192, 168, 1, 1 );
    IP4_ADDR ( &netmask, 255, 255, 255, 0 );
    init_lwip_timer();
    lwip_init();

    while ( ( netif_add ( &enc28j60_netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input ) == NULL ) ) {
        delay_ms ( 200 );
        delay_ms ( 200 );
    }

    netif_set_default ( &enc28j60_netif ); /* 注册默认的网络接口 */
    netif_set_up ( &enc28j60_netif ); /* 建立网络接口用于处理通信 */
    loop_test_init();
    tcpserver_init(); /* TCP服务器初始化 */
    loopclient_init();

    while ( 1 ) {
        LWIP_Polling();
        netif_poll ( &loop_netif ); /* 循环调用netif_poll函数，将链表上的数据包输入 */
    }
}