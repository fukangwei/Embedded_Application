#ifndef _SOCKET_H_
#define _SOCKET_H_
#include "sys.h"
#include "Types.h"

extern uint8 socket ( SOCKET s, uint8 protocol, uint16 port, uint8 flag );
extern void close ( SOCKET s );
extern uint8 connect ( SOCKET s, uint8 *addr, uint16 port );
extern void disconnect ( SOCKET s );
extern uint8 listen ( SOCKET s );
extern uint16 send ( SOCKET s, const uint8 *buf, uint16 len );
extern uint16 recv ( SOCKET s, uint8 *buf, uint16 len );
extern uint16 sendto ( SOCKET s, const uint8 *buf, uint16 len, uint8 *addr, uint16 port );
extern uint16 recvfrom ( SOCKET s, uint8 *buf, uint16 len, uint8 *addr, uint16  *port );

#ifdef __MACRAW__
    void macraw_open ( void );
    uint16 macraw_send ( const uint8 *buf, uint16 len );
    uint16 macraw_recv ( uint8 *buf, uint16 len );
#endif
#endif