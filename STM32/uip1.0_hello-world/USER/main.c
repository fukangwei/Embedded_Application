#include "delay.h"
#include "led.h"
#include "sys.h"
#include "uip.h"
#include "uip_arp.h"
#include "usart.h"
#include "tapdev.h"
#include "timer.h"
#include "enc28j60.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

void uip_polling ( void );

int main ( void ) {
    uip_ipaddr_t ipaddr;
    delay_init();
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    tapdev_init();
    uip_init(); /* uIP初始化 */
    uip_ipaddr ( ipaddr, 192, 168, 1, 230 ); /* 设置本地设置IP地址 */
    uip_sethostaddr ( ipaddr );
    uip_ipaddr ( ipaddr, 192, 168, 1, 1 ); /* 设置网关IP地址(路由器IP地址) */
    uip_setdraddr ( ipaddr );
    uip_ipaddr ( ipaddr, 255, 255, 255, 0 ); /* 设置网络掩码 */
    uip_setnetmask ( ipaddr );
    hello_world_init();

    while ( 1 ) {
        uip_polling(); /* 处理uip事件，必须插入到用户程序的循环体中 */
    }
}

void uip_polling ( void ) {
    u8 i;
    static struct timer periodic_timer, arp_timer;
    static u8 timer_ok = 0;

    if ( timer_ok == 0 ) { /* 仅初始化一次 */
        timer_ok = 1;
        timer_set ( &periodic_timer, CLOCK_SECOND / 2 ); /* 创建1个0.5秒的定时器 */
        timer_set ( &arp_timer, CLOCK_SECOND * 10 ); /* 创建1个10秒的定时器 */
    }

    uip_len = tapdev_read(); /* 从网络设备读取一个IP包，得到数据长度，uip_len在uip.c中定义 */

    if ( uip_len > 0 ) { /* 有数据 */
        /* 处理IP数据包(只有校验通过的IP包才会被接收) */
        if ( BUF->type == htons ( UIP_ETHTYPE_IP ) ) { /*是否为IP包 */
            uip_arp_ipin(); /* 去除以太网头结构，更新ARP表 */
            uip_input(); /* IP包处理 */

            if ( uip_len > 0 ) { /* 如果需要发送数据，则数据存储在uip_buf，长度是uip_len(这是2个全局变量) */
                uip_arp_out(); /* 加上以太网头结构，在主动连接时可能要构造ARP请求 */
                tapdev_send(); /* 发送数据到以太网 */
            }
        } else if ( BUF->type == htons ( UIP_ETHTYPE_ARP ) ) { /* 是否是ARP请求包 */
            uip_arp_arpin();

            if ( uip_len > 0 ) { /* 如果需要发送数据，则数据存储在uip_buf，长度是uip_len */
                tapdev_send(); /* 如果需要发送数据，则通过tapdev_send发送 */
            }
        }
    } else if ( timer_expired ( &periodic_timer ) ) { /* 0.5秒定时器超时 */
        timer_reset ( &periodic_timer ); /* 复位0.5秒定时器 */

        for ( i = 0; i < UIP_CONNS; i++ ) { /* 轮流处理每个TCP连接，UIP_CONNS缺省是40个 */
            uip_periodic ( i ); /* 处理TCP通信事件 */

            if ( uip_len > 0 ) { /* 如果需要发送数据，则数据存储在uip_buf，长度是uip_len(这是2个全局变量) */
                uip_arp_out(); /* 加上以太网头结构，在主动连接时可能要构造ARP请求 */
                tapdev_send(); /* 发送数据到以太网 */
            }
        }

#if UIP_UDP

        for ( i = 0; i < UIP_UDP_CONNS; i++ ) { /* 轮流处理每个UDP连接，UIP_UDP_CONNS缺省是10个 */
            uip_udp_periodic ( i ); /* 处理UDP通信事件 */

            if ( uip_len > 0 ) { /* 如果需要发送数据，则数据存储在uip_buf，长度是uip_len(这是2个全局变量) */
                uip_arp_out(); /* 加上以太网头结构，在主动连接时可能要构造ARP请求 */
                tapdev_send(); /* 发送数据到以太网 */
            }
        }

#endif

        if ( timer_expired ( &arp_timer ) ) { /* 每隔10秒调用1次ARP定时器函数，用于定期ARP处理 */
            timer_reset ( &arp_timer );
            uip_arp_timer();
        }
    }
}