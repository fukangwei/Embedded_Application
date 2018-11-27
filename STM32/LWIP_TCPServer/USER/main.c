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

#if 1  /* TCP������ */

static err_t tcpserver_recv ( void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err ) {
    if ( p != NULL ) { /* �����ݲ�Ϊ�� */
        tcp_recved ( pcb, p->tot_len ); /* ֪ͨ�ں˸��½��մ��� */
        tcp_write ( pcb, p->payload, p->len, 1 ); /* �������ֻ����һ��pbuf�����ݷ��ظ��ͻ��� */
        pbuf_free ( p ); /* �ͷ��������ݰ� */
    } else if ( err == ERR_OK ) { /* ����������մ��� */
        return tcp_close ( pcb ); /* �رո����� */
    }

    return ERR_OK; /* ���ز������ */
}

/* ���пͻ������������������ʱ���ú����ᱻϵͳ�ص�ִ�� */
static err_t tcpserver_acccept ( void *arg, struct tcp_pcb *pcb, err_t err ) {
    tcp_recv ( pcb, tcpserver_recv ); /* �������ע�ắ��tcpserver_recv */
    return ERR_OK;
}

void tcpserver_init ( void ) {
    struct tcp_pcb *pcb;
    pcb = tcp_new(); /* ����һ��TCP���ƿ� */
    tcp_bind ( pcb, IP_ADDR_ANY, 8080 ); /* ���ÿ��ƿ�ͱ��ض˿�8080�� */
    pcb = tcp_listen ( pcb ); /* ���ƿ�Ϊ����״̬���ȴ��ͻ��˵����� */
    tcp_accept ( pcb, tcpserver_acccept ); /* ע�����ӻص�����tcpserver_accept�����пͻ������ӷ�����ʱ���ú�����ϵͳ�ص� */
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
    tcpserver_init(); /* TCP��������ʼ�� */

    while ( 1 ) {
        LWIP_Polling();
    }
}