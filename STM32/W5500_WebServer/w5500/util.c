#include "config.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef __GNUC__
    /* With GCC/RAISONANCE, small printf (option LD Linker -> Libraries ->
    Small printf set to 'Yes') calls __io_putchar() */
    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
    #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

static u8  fac_us = 0;
static u16 fac_ms = 0;

void Systick_Init ( u8 SYSCLK ) {
    SysTick->CTRL &= 0xfffffffb;
    fac_us = SYSCLK / 8;
    fac_ms = ( u16 ) fac_us * 1000;
}

void Delay_s ( uint32 time_s ) {
    for ( ; time_s > 0; time_s-- ) {
        Delay_ms ( 1000 );
    }
}

void Delay_ms ( uint32 time_ms ) {
    u32 temp;
    SysTick->LOAD = ( u32 ) time_ms * fac_ms;
    SysTick->VAL = 0x00;
    SysTick->CTRL = 0x01 ;

    do {
        temp = SysTick->CTRL;
    } while ( temp & 0x01 && ! ( temp & ( 1 << 16 ) ) );

    SysTick->CTRL = 0x00;
    SysTick->VAL = 0X00;
}

void Delay_us ( uint32 time_us ) {
    u32 temp;
    SysTick->LOAD = time_us * fac_us;
    SysTick->VAL = 0x00;
    SysTick->CTRL = 0x01;

    do {
        temp = SysTick->CTRL;
    } while ( temp & 0x01 && ! ( temp & ( 1 << 16 ) ) );

    SysTick->CTRL = 0x00;
    SysTick->VAL = 0X00;
}

uint16 ATOI ( /* CONVERT STRING INTO INTEGER */
    char *str, /* is a pointer to convert */
    uint16 base /* is a base value (must be in the range 2 - 16) */
) {
    unsigned int num = 0;

    while ( *str != 0 ) {
        num = num * base + C2D ( *str++ );
    }

    return num;
}

uint32 ATOI32 (
    char *str, /* is a pointer to convert */
    uint16 base /* is a base value (must be in the range 2 - 16) */
) {
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

int ValidATOI ( /* CONVERT STRING INTO HEX OR DECIMAL */
    char *str, /* is a pointer to string to be converted */
    int base, /* is a base value (must be in the range 2 - 16) */
    int *ret /*  is a integer pointer to return */
) {
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

void replacetochar ( /* replace the specified character in a string with new character */
    char *str, /* pointer to be replaced */
    char oldchar, /* old character */
    char newchar /* new character */
) {
    int x;

    for ( x = 0; str[x]; x++ )
        if ( str[x] == oldchar ) {
            str[x] = newchar;
        }
}

char C2D ( /* CONVERT CHAR INTO HEX */
    uint8 c /* is a character('0'-'F') to convert to HEX */
) {
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

void mid ( int8 *src, int8 *s1, int8 *s2, int8 *sub ) { /* get mid str */
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
/*******************************************************************************
* Function Name : assert_failed
* Description   : Reports the name of the source file and the source line number
*                 where the assert_param error has occurred.
* Input         : file: pointer to the source file name
*                 line: assert_param error line source number
*******************************************************************************/
void assert_failed ( uint8_t *file, uint32_t line ) {
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    while ( 1 ) { /* Infinite loop */
    }
}
#endif
