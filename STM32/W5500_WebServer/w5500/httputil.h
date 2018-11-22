#ifndef __HTTPUTIL_H__
#define __HTTPUTIL_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"
#include "w5500.h"
#include "config.h"
#include "util.h"
#include "device.h"
#include "httpd.h"
#include "socket.h"
#include "sockutil.h"
#include "util.h"

void proc_http ( SOCKET s, u_char *buf );
void do_http ( void );
void cgi_ipconfig ( st_http_request *http_request );
uint16 make_msg_response ( uint8 *buf, int8 *msg );
void make_cgi_response ( uint16 a, int8 *b, int8 *c );
void make_pwd_response ( int8 isRight, uint16 delay, int8 *cgi_response_content, int8 isTimeout );
#endif