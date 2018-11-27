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

/* 这个程序在使用Windows自带的Telnet客户端是可以正常工作的，在使用SecureCRT是不能正常工作的 */
/* 定义三个全局变量，sndbuf用于保存向客户端返回的字符串结果，cmdbuf用于记录客户端的命令，cmd_flag用于记录当前客户端命令的长度 */
char sndbuf[50] = {0}, cmdbuf[20] = {0};
s8_t cmd_flag = 0;

static err_t telnet_recv ( void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err ) { /* 服务器接收到数据后的回调函数 */
    if ( p != NULL ) { /* 收到的报文包含数据 */
        u16_t len;
        u8_t *datab;
        u16_t strlen;
        tcp_recved ( pcb, p->tot_len ); /* 通知内核更新接收窗口 */
        len = p->len; /* 获得数据长度 */
        datab = ( unsigned char * ) p->payload; /* 获得数据起始地址 */

        if ( ( len == 2 ) && ( *datab == 0x0d ) && ( * ( datab + 1 ) == 0x0a ) ) { /* 如果收到的是回车字符，则结束用户命令的读取，解析命令并返回处理结果 */
            if ( cmd_flag > 0 ) { /* 如果用户已输入命令大于0，则解析命令 */
                cmdbuf[cmd_flag] = 0x00;  /* 命令字符串结束 */

                if ( strcmp ( cmdbuf, "date" ) == 0 ) { /* 如果是date命令，则向用户返回如下字符串 */
                    strlen = sprintf ( sndbuf, "Now, It is 2016-xx-xx!\r\n" );
                    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY ); /* 发送时拷贝 */
                } else if ( strcmp ( cmdbuf, "hello" ) == 0 ) { /* 如果是hello命令，则向用户返回如下字符串 */
                    strlen = sprintf ( sndbuf, "Hello, Nice to see you!\r\n" );
                    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
                } else if ( strcmp ( cmdbuf, "more" ) == 0 ) { /* 如果是more命令，则向用户返回如下字符串 */
                    strlen = sprintf ( sndbuf, "Add whatever you need in this way!\r\n" );
                    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
                } else if ( strcmp ( cmdbuf, "help" ) == 0 ) { /* 如果是help命令，则向用户返回如下字符串 */
                    strlen = sprintf ( sndbuf, "supported Command: date hello more help quit\r\n" );
                    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
                } else if ( strcmp ( cmdbuf, "quit" ) == 0 ) { /* 如果是quit命令 */
                    cmd_flag = 0; /* 用户命令标志清0 */
                    pbuf_free ( p ); /* 释放数据pbuf */
                    return tcp_close ( pcb ); /* 服务器主动关闭连接，返回 */
                } else { /* 对于其他命令，向用户返回如下提示信息 */
                    strlen = sprintf ( sndbuf, "Unknown Command: %s\r\n", cmdbuf );
                    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
                }

                cmd_flag = 0; /* 用户命令标志清0，接收下一个命令 */
            }

            strlen = sprintf ( sndbuf, "\r\nForrest_Shell>>" ); /* 向用户发送提示符 */
            tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
        } else if ( ( len == 1 ) && ( *datab >= 0x20 ) && ( *datab <= 0x7e ) && ( cmd_flag < 19 ) ) { /* 如果收到的是字符，则在命令缓冲中记录字符 */
            cmdbuf[cmd_flag] = *datab;
            cmd_flag++; /* 命令标志加1 */
        } else if ( ( len == 1 ) && ( *datab == 0x08 ) && ( cmd_flag > 0 ) ) { /* 如果收到的是删除字符且命令缓冲中还有字符，则在缓冲中删除一个字符 */
            cmd_flag--; /* 删除一个字符 */
            strlen = sprintf ( sndbuf, "\b\b" ); /* 向用户返回退格，调整用户的命令行显示 */
            tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
        } else if ( ( len == 1 ) && ( *datab == 0x08 ) ) {
            /* 命令删除完毕，则需要调整提示符显示 */
            cmd_flag = 0;
            strlen = sprintf ( sndbuf, ">" );
            tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
        }

        pbuf_free ( p ); /* 删除收到的报文段 */
    } else { /* 若数据为空，表示收到对方的FIN，则关闭连接 */
        cmd_flag = 0;
        return tcp_close ( pcb );
    }

    return ERR_OK;
}

static err_t telnet_accept ( void *arg, struct tcp_pcb *pcb, err_t err ) { /* 服务器accept回调函数，当有连接建立后，该函数被回调 */
    u16_t strlen;
    tcp_recv ( pcb, telnet_recv ); /* 注册recv函数 */
    /* 连接建立后，向客户端发送一些Telnet相关的字符串信息 */
    strlen = sprintf ( sndbuf, "##Welcome to demo TELNET based on LwIP##\r\n" );
    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
    strlen = sprintf ( sndbuf, "##Created by Forrest...##\r\n" );
    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
    strlen = sprintf ( sndbuf, "##quit:退出  help:帮助信息##\r\n" );
    tcp_write ( pcb, sndbuf, strlen, TCP_WRITE_FLAG_COPY );
    strlen = sprintf ( sndbuf, "Forrest_Shell>>" );
    tcp_write ( pcb, sndbuf, strlen, 1 );
    return ERR_OK;
}

void telnet_init ( void ) { /* Telnet服务器初始化程序 */
    struct tcp_pcb *pcb;
    pcb = tcp_new(); /* 新建一个控制块 */
    tcp_bind ( pcb, IP_ADDR_ANY, 23 ); /* 绑定本地熟知端口号23(Telnet) */
    pcb = tcp_listen ( pcb ); /* 服务器进入侦听状态，等到客户端的连接 */
    tcp_accept ( pcb, telnet_accept ); /* 注册建立连接后的回调函数 */
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