#include "httputil.h"
#include "device.h"

#define DEBUG_HTTP

extern CONFIG_MSG  ConfigMsg;
extern char tx_buf[MAX_URI_SIZE];
extern char rx_buf[MAX_URI_SIZE];

uint8 boundary[64];
uint8 tmp_buf[1460] = {0xff,};

void do_http ( void ) {
    uint8 ch = SOCK_HTTP;
    uint16 len;
    st_http_request *http_request;
    memset ( rx_buf, 0x00, MAX_URI_SIZE );
    http_request = ( st_http_request * ) rx_buf; /* struct of http request */

    switch ( getSn_SR ( ch ) ) { /* http service start */
        case SOCK_INIT:
            listen ( ch );
            break;

        case SOCK_LISTEN:
            break;

        case SOCK_ESTABLISHED:
            if ( getSn_IR ( ch ) & Sn_IR_CON ) {
                setSn_IR ( ch, Sn_IR_CON );
            }

            if ( ( len = getSn_RX_RSR ( ch ) ) > 0 ) {
                len = recv ( ch, ( uint8 * ) http_request, len );
                * ( ( ( uint8 * ) http_request ) + len ) = 0;
                proc_http ( ch, ( uint8 * ) http_request ); /* request is processed */
                disconnect ( ch );
            }

            break;

        case SOCK_CLOSE_WAIT:
            if ( ( len = getSn_RX_RSR ( ch ) ) > 0 ) {
                len = recv ( ch, ( uint8 * ) http_request, len );
                * ( ( ( uint8 * ) http_request ) + len ) = 0;
                proc_http ( ch, ( uint8 * ) http_request ); /* request is processed */
            }

            disconnect ( ch );
            break;

        case SOCK_CLOSED:
            socket ( ch, Sn_MR_TCP, 80, 0x00 ); /* reinitialize the socket */
            break;

        default:
            break;
    }
}

void proc_http ( SOCKET s, uint8 *buf ) {
    int8 *name; /* get method request file name */
    uint8 *http_response;
    st_http_request *http_request;
    memset ( tx_buf, 0x00, MAX_URI_SIZE );
    http_response = ( uint8 * ) rx_buf;
    http_request = ( st_http_request * ) tx_buf;
    parse_http_request ( http_request, buf ); /* After analyze request, convert into http_request */

    switch ( http_request->METHOD ) { /* method analyze */
        case METHOD_ERR :
            printf ( "e" );

            if ( strlen ( ( int8 const * ) boundary ) > 0 ) {
                printf ( "Error=%s\r\n", http_request->URI );
            } else {
                memcpy ( http_response, ERROR_REQUEST_PAGE, sizeof ( ERROR_REQUEST_PAGE ) );
                send ( s, ( uint8 * ) http_response, strlen ( ( int8 const * ) http_response ) );
            }

            break;

        case METHOD_HEAD:
        case METHOD_GET:
            name = http_request->URI;

            if ( strcmp ( name, "/index.htm" ) == 0 || strcmp ( name, "/" ) == 0 || ( strcmp ( name, "/index.html" ) == 0 ) ) {
                memset ( tx_buf, 0, MAX_URI_SIZE );
                sprintf ( tx_buf, "<html><head><title>Hello!WIZnet!</title></head><body><h1 align='center'>Hello!Fukangwei!</h1></body></html>" );
                sprintf ( ( char * ) http_response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s", strlen ( tx_buf ), tx_buf );
                send ( s, ( u_char * ) http_response, strlen ( ( char const * ) http_response ) );
            }

            break;

        default :
            break;
    }
}