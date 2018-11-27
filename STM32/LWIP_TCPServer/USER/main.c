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

#define CLOCKTICKS_PER_MS 10

static ip_addr_t ipaddr, netmask, gw;
struct netif enc28j60_netif;
u32_t last_arp_time;
u32_t last_tcp_time;
u32_t last_ipreass_time;

u32_t input_time;

#if 1  /* TCP服务器 */

static err_t tcpserver_recv ( void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err ) {
    if ( p != NULL ) { /* 当数据不为空 */
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

#if LWIP_DHCP > 0

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

    netif_set_default ( &enc28j60_netif );
    netif_set_up ( &enc28j60_netif );
    tcpserver_init(); /* TCP服务器初始化 */

    while ( 1 ) {
        LWIP_Polling();
    }
}