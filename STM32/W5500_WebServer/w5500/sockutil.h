#ifndef __SOCKUTIL_H
#define __SOCKUTIL_H

#define NO_USE_SOCKUTIL_FUNC

char *inet_ntoa ( unsigned long addr );
char *inet_ntoa_pad ( unsigned long addr );
char VerifyIPAddress_orig ( char *src );
char VerifyIPAddress ( char *src, uint8 *ip );
unsigned long GetDestAddr ( SOCKET s );
unsigned int GetDestPort ( SOCKET s );
unsigned short htons ( unsigned short hostshort );
unsigned long htonl ( unsigned long hostlong );
unsigned long ntohs ( unsigned short netshort );
unsigned long ntohl ( unsigned long netlong );
u_char CheckDestInLocal ( u_long destip );
SOCKET getSocket ( unsigned char status, SOCKET start );
unsigned short checksum ( unsigned char *src, unsigned int len );

#ifndef NO_USE_SOCKUTIL_FUNC
    u_long GetIPAddress ( void );
    u_long GetGWAddress ( void );
    u_long GetSubMask ( void );
    void GetMacAddress ( unsigned char *mac );
    void GetDestMacAddr ( SOCKET s, u_char *mac );
    void GetNetConfig ( void );
    void dump_iinchip ( void );
#endif
#endif