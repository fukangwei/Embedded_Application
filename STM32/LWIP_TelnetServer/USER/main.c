#include "delay.h"
#include "led.h"
#include "mysys.h"
#include "usart.h"
#include "enc28j60.h"
#include "string.h"
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

#define TELNET_SERVER 1

#if TELNET_SERVER

/* ���������ʹ��Windows�Դ���Telnet�ͻ����ǿ������������ģ���ʹ��SecureCRT�ǲ������������� */
/* ��������ȫ�ֱ�����sndbuf���ڱ�����ͻ��˷��ص��ַ��������cmdbuf���ڼ�¼�ͻ��˵����cmd_flag���ڼ�¼��ǰ�ͻ�������ĳ��� */
char sndbuf[50] = {0}, cmdbuf[20] = {0};
s8_t cmd_flag = 0;

static err_t telnet_recv ( void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err ) { /* ���������յ����ݺ�Ļص����� */
    if ( p != NULL ) { /* �յ��ı��İ������� */
        u16_t len;
        u8_t *datab;
        u16_t strlen;
        tcp_recved ( pcb, p->tot_len ); /* ֪ͨ�ں˸��½��մ��� */
        len = p->len; /* ������ݳ��� */
        datab = ( unsigned char * ) p->payload; /* ���������ʼ��ַ */

        if ( ( len == 2 ) && ( *datab == 0x0d ) && ( * ( datab + 1 ) == 0x0a ) ) { /* ����յ����ǻس��ַ���������û�����Ķ�ȡ������������ش����� */
            if ( cmd_flag > 0 ) { /* ����û��������������0����������� */
                cmdbuf[cmd_flag] = 0x00;  /* �����ַ������� */

                if ( strcmp ( cmdbuf, "date" ) == 0 ) { /* �����date��������û����������ַ��� */
                    strlen = sprintf ( sndbuf, "Now, It is 2016-xx-xx!\r\n" );
                    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY ); /* ����ʱ���� */
                } else if ( strcmp ( cmdbuf, "hello" ) == 0 ) { /* �����hello��������û����������ַ��� */
                    strlen = sprintf ( sndbuf, "Hello, Nice to see you!\r\n" );
                    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
                } else if ( strcmp ( cmdbuf, "more" ) == 0 ) { /* �����more��������û����������ַ��� */
                    strlen = sprintf ( sndbuf, "Add whatever you need in this way!\r\n" );
                    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
                } else if ( strcmp ( cmdbuf, "help" ) == 0 ) { /* �����help��������û����������ַ��� */
                    strlen = sprintf ( sndbuf, "supported Command: date hello more help quit\r\n" );
                    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
                } else if ( strcmp ( cmdbuf, "quit" ) == 0 ) { /* �����quit���� */
                    cmd_flag = 0; /* �û������־��0 */
                    pbuf_free ( p ); /* �ͷ�����pbuf */
                    return tcp_close ( pcb ); /* �����������ر����ӣ����� */
                } else { /* ��������������û�����������ʾ��Ϣ */
                    strlen = sprintf ( sndbuf, "Unknown Command: %s\r\n", cmdbuf );
                    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
                }

                cmd_flag = 0; /* �û������־��0��������һ������ */
            }

            strlen = sprintf ( sndbuf, "\r\nForrest_Shell>>" ); /* ���û�������ʾ�� */
            tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
        } else if ( ( len == 1 ) && ( *datab >= 0x20 ) && ( *datab <= 0x7e ) && ( cmd_flag < 19 ) ) { /* ����յ������ַ�������������м�¼�ַ� */
            cmdbuf[cmd_flag] = *datab;
            cmd_flag++; /* �����־��1 */
        } else if ( ( len == 1 ) && ( *datab == 0x08 ) && ( cmd_flag > 0 ) ) { /* ����յ�����ɾ���ַ���������л����ַ������ڻ�����ɾ��һ���ַ� */
            cmd_flag--; /* ɾ��һ���ַ� */
            strlen = sprintf ( sndbuf, "\b\b" ); /* ���û������˸񣬵����û�����������ʾ */
            tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
        } else if ( ( len == 1 ) && ( *datab == 0x08 ) ) {
            /* ����ɾ����ϣ�����Ҫ������ʾ����ʾ */
            cmd_flag = 0;
            strlen = sprintf ( sndbuf, ">" );
            tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
        }

        pbuf_free ( p ); /* ɾ���յ��ı��Ķ� */
    } else { /* ������Ϊ�գ���ʾ�յ��Է���FIN����ر����� */
        cmd_flag = 0;
        return tcp_close ( pcb );
    }

    return ERR_OK;
}

static err_t telnet_accept ( void *arg, struct tcp_pcb *pcb, err_t err ) { /* ������accept�ص��������������ӽ����󣬸ú������ص� */
    u16_t strlen;
    tcp_recv ( pcb, telnet_recv ); /* ע��recv���� */
    /* ���ӽ�������ͻ��˷���һЩTelnet��ص��ַ�����Ϣ */
    strlen = sprintf ( sndbuf, "##Welcome to demo TELNET based on LwIP##\r\n" );
    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
    strlen = sprintf ( sndbuf, "##Created by Forrest...##\r\n" );
    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
    strlen = sprintf ( sndbuf, "##quit:�˳�  help:������Ϣ##\r\n" );
    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
    strlen = sprintf ( sndbuf, "Forrest_Shell>>" );
    tcp_write ( pcb, sndbuf, strlen, 1 );
    return ERR_OK;
}

void telnet_init ( void ) { /* Telnet��������ʼ������ */
    struct tcp_pcb *pcb;
    pcb = tcp_new(); /* �½�һ�����ƿ� */
    tcp_bind ( pcb, IP_ADDR_ANY, 23 ); /* �󶨱�����֪�˿ں�23(Telnet) */
    pcb = tcp_listen ( pcb ); /* ��������������״̬���ȵ��ͻ��˵����� */
    tcp_accept ( pcb, telnet_accept ); /* ע�Ὠ�����Ӻ�Ļص����� */
}

#endif

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
    telnet_init();

    while ( 1 ) {
        LWIP_Polling();
    }
}