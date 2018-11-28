#include "lwip_demo.h"
#include "usart.h"

u8 lwip_demo_buf[LWIP_DEMO_BUF];
u8 lwip_flag;

u8 lwip_test_mode = LWIP_TCP_SERVER;

void lwip_demo_init ( void ) {
}

void lwip_log ( char *str ) {
#if LWIP_DEMO_DEBUG>0
    printf ( "lwip:%s\r\n", str );
#endif
}