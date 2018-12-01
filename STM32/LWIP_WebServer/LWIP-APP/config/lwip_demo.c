#include "lwip_demo.h"

#include "httpd.h"
#include "ssi_cgi_handle.h"
#include "usart.h"

u8 lwip_demo_buf[LWIP_DEMO_BUF];
u8 lwip_flag;

u8 lwip_test_mode = LWIP_TCP_SERVER;

void lwip_demo_init ( void ) {
    switch ( lwip_test_mode ) {
        case LWIP_TCP_SERVER:
            Init_TCP_Server();
            break;

        case LWIP_TCP_CLIENT:
            Init_TCP_Client();
            break;

        case LWIP_UDP_SERVER:
            Init_UDP_Server();
            break;

        case LWIP_UDP_CLIENT:
            Init_UDP_Client();
            break;

        case LWIP_WEBSERVER:
            httpd_init();
            init_ssi_cgi();
            break;

        default:
            break;
    }
}

void lwip_log ( char *str ) {
#if LWIP_DEMO_DEBUG > 0
    printf ( "lwip:%s\r\n", str );
#endif
}