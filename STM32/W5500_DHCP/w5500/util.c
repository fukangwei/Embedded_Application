#include "config.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef __GNUC__
    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
    #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

uint16 ATOI ( char *str, uint16 base ) {
    unsigned int num = 0;

    while ( *str != 0 ) {
        num = num * base + C2D ( *str++ );
    }

    return num;
}

uint32 ATOI32 ( char *str, uint16 base ) {
    uint32 num = 0;

    while ( *str != 0 ) {
        num = num * base + C2D ( *str++ );
    }

    return num;
}

void itoa ( uint16 n, uint8 str[5], uint8 len ) {
    uint8 i = len - 1;
    memset ( str, 0x20, len );

    do {
        str[i--] = n % 10 + '0';
    } while ( ( n /= 10 ) > 0 );

    return;
}

int ValidATOI ( char *str, int base, int *ret ) {
    int c;
    char *tstr = str;

    if ( str == 0 || *str == '\0' ) {
        return 0;
    }

    while ( *tstr != '\0' ) {
        c = C2D ( *tstr );

        if ( c >= 0 && c < base ) {
            tstr++;
        } else {
            return 0;
        }
    }

    *ret = ATOI ( str, base );
    return 1;
}

void replacetochar ( char *str, char oldchar, char newchar ) {
    int x;

    for ( x = 0; str[x]; x++ )
        if ( str[x] == oldchar ) {
            str[x] = newchar;
        }
}

char C2D ( uint8 c ) {
    if ( c >= '0' && c <= '9' ) {
        return c - '0';
    }

    if ( c >= 'a' && c <= 'f' ) {
        return 10 + c - 'a';
    }

    if ( c >= 'A' && c <= 'F' ) {
        return 10 + c - 'A';
    }

    return ( char ) c;
}

uint16 swaps ( uint16 i ) {
    uint16 ret = 0;
    ret = ( i & 0xFF ) << 8;
    ret |= ( ( i >> 8 ) & 0xFF );
    return ret;
}

uint32 swapl ( uint32 l ) {
    uint32 ret = 0;
    ret = ( l & 0xFF ) << 24;
    ret |= ( ( l >> 8 ) & 0xFF ) << 16;
    ret |= ( ( l >> 16 ) & 0xFF ) << 8;
    ret |= ( ( l >> 24 ) & 0xFF );
    return ret;
}

void mid ( int8 *src, int8 *s1, int8 *s2, int8 *sub ) {
    int8 *sub1;
    int8 *sub2;
    uint16 n;
    sub1 = strstr ( src, s1 );
    sub1 += strlen ( s1 );
    sub2 = strstr ( sub1, s2 );
    n = sub2 - sub1;
    strncpy ( sub, sub1, n );
    sub[n] = 0;
}

void inet_addr_ ( unsigned char *addr, unsigned char *ip ) {
    int i;
    char taddr[30];
    char *nexttok;
    char num;
    strcpy ( taddr, ( char * ) addr );
    nexttok = taddr;

    for ( i = 0; i < 4 ; i++ ) {
        nexttok = strtok ( nexttok, "." );

        if ( nexttok[0] == '0' && nexttok[1] == 'x' ) {
            num = ATOI ( nexttok + 2, 0x10 );
        } else {
            num = ATOI ( nexttok, 10 );
        }

        ip[i] = num;
        nexttok = NULL;
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed ( uint8_t *file, uint32_t line ) {
    while ( 1 ) {
    }
}
#endif