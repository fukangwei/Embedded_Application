#ifndef  __LWIP_DEMO_H
#define  __LWIP_DEMO_H
#include "mysys.h"
#include "udp_server.h"
#include "udp_client.h"


#define LWIP_CONNECTED  0X80
#define LWIP_NEW_DATA   0x40
#define LWIP_SEND_DATA  0x20
#define LWIP_DEMO_BUF   200

#define LWIP_TCP_SERVER 0x80
#define LWIP_TCP_CLIENT 0x40
#define LWIP_UDP_SERVER 0x20
#define LWIP_UDP_CLIENT 0x10
#define LWIP_WEBSERVER  0x08

#define LWIP_DEMO_DEBUG 1

extern u8 lwip_flag;


extern u8 lwip_test_mode;
extern u8 lwip_demo_buf[];

void lwip_demo_init ( void );
void lwip_log ( char *str );

#endif