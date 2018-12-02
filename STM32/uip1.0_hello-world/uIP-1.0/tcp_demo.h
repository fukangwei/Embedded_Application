#ifndef __TCP_DEMO_H__
#define __TCP_DEMO_H__
#include "uipopt.h"
#include "psock.h"
#include "sys.h"

enum { /* 通信程序状态字(用户可以自己定义) */
    STATE_CMD     = 0, /* 命令接收状态 */
    STATE_TX_TEST = 1, /* 连续发送数据包状态(速度测试) */
    STATE_RX_TEST = 2  /* 连续接收数据包状态(速度测试) */
};

struct tcp_demo_appstate {
    u8_t state;
    u8_t *textptr;
    int textlen;
};

typedef struct tcp_demo_appstate uip_tcp_appstate_t;

void tcp_demo_appcall ( void );
void tcp_client_demo_appcall ( void );
void tcp_server_demo_appcall ( void );

#ifndef UIP_APPCALL
    #define UIP_APPCALL tcp_demo_appcall
#endif

extern u8 tcp_server_databuf[];
extern u8 tcp_server_sta;
extern u8 tcp_client_databuf[];
extern u8 tcp_client_sta;

void tcp_server_aborted ( void );
void tcp_server_timedout ( void );
void tcp_server_closed ( void );
void tcp_server_connected ( void );
void tcp_server_newdata ( void );
void tcp_server_acked ( void );
void tcp_server_senddata ( void );
void tcp_client_reconnect ( void );
void tcp_client_connected ( void );
void tcp_client_aborted ( void );
void tcp_client_timedout ( void );
void tcp_client_closed ( void );
void tcp_client_acked ( void );
void tcp_client_senddata ( void );
#endif