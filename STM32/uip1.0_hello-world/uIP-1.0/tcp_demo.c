#include "sys.h"
#include "usart.h"
#include "uip.h"
#include "enc28j60.h"
#include "tcp_demo.h"

void tcp_demo_appcall ( void ) {
    switch ( uip_conn->lport ) {
        case HTONS ( 80 ) :
            break;

        case HTONS ( 1200 ) :
            break;

        default:
            break;
    }

    switch ( uip_conn->rport ) {
        case HTONS ( 1400 ) :
            break;

        default:
            break;
    }
}

void uip_log ( char *m ) {
    printf ( "uIP log:%s\r\n", m );
}