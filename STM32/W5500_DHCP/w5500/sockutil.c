#include <stdio.h>
#include <string.h>
#include "types.h"
#include "w5500.h"
#include "util.h"
#include "sockutil.h"

char *inet_ntoa ( unsigned long addr ) {
    static char addr_str[32];
    memset ( addr_str, 0, 32 );
    sprintf ( addr_str, "%d.%d.%d.%d", ( int ) ( addr >> 24 & 0xFF ), ( int ) ( addr >> 16 & 0xFF ),
            ( int ) ( addr >> 8 & 0xFF ), ( int ) ( addr & 0xFF ) );
    return addr_str;
}

char *inet_ntoa_pad ( unsigned long addr ) {
    static char addr_str[16];
    memset ( addr_str, 0, 16 );
    printf ( addr_str, "%03d.%03d.%03d.%03d", ( int ) ( addr >> 24 & 0xFF ), ( int ) ( addr >> 16 & 0xFF ),
           ( int ) ( addr >> 8 & 0xFF ), ( int ) ( addr & 0xFF ) );
    return addr_str;
}

char VerifyIPAddress_orig ( char *src ) {
    int i;
    int tnum;
    char tsrc[50];
    char *tok = tsrc;
    strcpy ( tsrc, src );

    for ( i = 0; i < 4; i++ ) {
        tok = strtok ( tok, "." );

        if ( !tok ) {
            return 0;
        }

        if ( tok[0] == '0' && tok[1] == 'x' ) {
            if ( !ValidATOI ( tok + 2, 0x10, &tnum ) ) {
                return 0;
            }
        } else if ( !ValidATOI ( tok, 10, &tnum ) ) {
            return 0;
        }

        if ( tnum < 0 || tnum > 255 ) {
            return 0;
        }

        tok = NULL;
    }

    return 1;
}

char VerifyIPAddress ( char *src, uint8 *ip ) {
    int i;
    int tnum;
    char tsrc[50];
    char *tok = tsrc;
    strcpy ( tsrc, src );

    for ( i = 0; i < 4; i++ ) {
        tok = strtok ( tok, "." );

        if ( !tok ) {
            return 0;
        }

        if ( tok[0] == '0' && tok[1] == 'x' ) {
            if ( !ValidATOI ( tok + 2, 0x10, &tnum ) ) {
                return 0;
            }
        } else if ( !ValidATOI ( tok, 10, &tnum ) ) {
            return 0;
        }

        ip[i] = tnum;

        if ( tnum < 0 || tnum > 255 ) {
            return 0;
        }

        tok = NULL;
    }

    return 1;
}

unsigned long GetDestAddr ( SOCKET s ) {
    u_long addr = 0;
    int i = 0;

    for ( i = 0; i < 4; i++ ) {
        addr <<= 8;
        addr += IINCHIP_READ ( Sn_DIPR0 ( s ) + i );
    }

    return addr;
}

unsigned int GetDestPort ( SOCKET s ) {
    u_int port;
    port = ( ( u_int ) IINCHIP_READ ( Sn_DPORT0 ( s ) ) ) & 0x00FF;
    port <<= 8;
    port += ( ( u_int ) IINCHIP_READ ( Sn_DPORT0 ( s ) + 1 ) ) & 0x00FF;
    return port;
}

uint16 htons ( uint16 hostshort ) {
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
    return swaps ( hostshort );
#else
    return hostshort;
#endif
}

unsigned long htonl ( unsigned long hostlong ) {
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
    return swapl ( hostlong );
#else
    return hostlong;
#endif
}

unsigned long ntohs ( unsigned short netshort ) {
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
    return htons ( netshort );
#else
    return netshort;
#endif
}

unsigned long ntohl ( unsigned long netlong ) {
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
    return htonl ( netlong );
#else
    return netlong;
#endif
}

u_char CheckDestInLocal ( u_long destip ) {
    int i = 0;
    u_char *pdestip = ( u_char * ) &destip;

    for ( i = 0; i < 4; i++ ) {
        if ( ( pdestip[i] & IINCHIP_READ ( SUBR0 + i ) ) != ( IINCHIP_READ ( SIPR0 + i ) & IINCHIP_READ ( SUBR0 + i ) ) ) {
            return 1;
        }
    }

    return 0;
}

SOCKET getSocket ( unsigned char status, SOCKET start ) {
    SOCKET i;

    if ( start > 3 ) {
        start = 0;
    }

    for ( i = start; i < MAX_SOCK_NUM ; i++ ) if ( getSn_SR ( i ) == status ) {
            return i;
        }

    return MAX_SOCK_NUM;
}

unsigned short checksum ( unsigned char *src, unsigned int len ) {
    u_int sum, tsum, i, j;
    u_long lsum;
    j = len >> 1;
    lsum = 0;

    for ( i = 0; i < j; i++ ) {
        tsum = src[i * 2];
        tsum = tsum << 8;
        tsum += src[i * 2 + 1];
        lsum += tsum;
    }

    if ( len % 2 ) {
        tsum = src[i * 2];
        lsum += ( tsum << 8 );
    }

    sum = lsum;
    sum = ~ ( sum + ( lsum >> 16 ) );
    return ( u_short ) sum;
}

#ifndef NO_USE_SOCKUTIL_FUNC
u_long GetIPAddress ( void ) {
    u_long ip = 0;
    int i;

    for ( i = 0; i < 4; i++ ) {
        ip <<= 8;
        ip += ( char ) IINCHIP_READ ( SIPR0 + i );
    }

    return ip;
}

u_long GetGWAddress ( void ) {
    u_long ip = 0;
    int i;

    for ( i = 0; i < 4; i++ ) {
        ip <<= 8;
        ip += ( char ) IINCHIP_READ ( GAR0 + i );
    }

    return ip;
}

u_long GetSubMask ( void ) {
    u_long ip = 0;
    int i;

    for ( i = 0; i < 4; i++ ) {
        ip <<= 8;
        ip += ( char ) IINCHIP_READ ( SUBR0 + i );
    }

    return ip;
}

void GetMacAddress ( unsigned char *mac ) {
    int i = 0;

    for ( i = 0; i < 6; i++ ) {
        *mac++ = IINCHIP_READ ( SHAR0 + i );
    }
}

void GetDestMacAddr ( SOCKET s, u_char *mac ) {
    int i = 0;

    for ( i = 0; i < 6; i++ ) {
        *mac++ = IINCHIP_READ ( Sn_DHAR0 ( s ) + i );
    }
}

void GetNetConfig ( void ) {
    u_char addr[6];
    u_long iaddr;
    printf ( "\r\n================================================\r\n" );
    printf ( "       Net Config Information\r\n" );
    printf ( "================================================\r\n" );
    GetMacAddress ( addr );
    printf ( "MAC ADDRESS      : 0x%02X.0x%02X.0x%02X.0x%02X.0x%02X.0x%02X\r\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5] );
    iaddr = GetSubMask();
    printf ( "SUBNET MASK      : %s\r\n", inet_ntoa ( iaddr ) );
    iaddr = GetGWAddress();
    printf ( "G/W IP ADDRESS   : %s\r\n", inet_ntoa ( iaddr ) );
    iaddr = GetIPAddress();
    printf ( "LOCAL IP ADDRESS : %s\r\n", inet_ntoa ( iaddr ) );
    printf ( "================================================\r\n" );
}
#endif