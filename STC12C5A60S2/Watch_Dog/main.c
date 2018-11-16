#include "STC12C5A.h"

typedef unsigned char uint8;
typedef unsigned int  uint16;

sbit DU = P2 ^ 6;
sbit WE = P2 ^ 7;

uint8 code table[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x40};

void delayms ( uint8 ms ) {
    uint8 i, j;

    for ( i = ms; i > 0; i-- )
        for ( j = 110; j > 0; j-- );
}

void delay500ms ( void ) {
    unsigned char a, b, c;

    for ( c = 246; c > 0; c-- )
        for ( b = 212; b > 0; b-- )
            for ( a = 25; a > 0; a-- );
}

void SmgShow ( uint8 num ) {
    DU = 1;
    P0 = table[num];
    DU = 0;
    P0 = 0xff;
    WE = 1;
    P0 = 0xdf;
    WE = 0;
    delayms ( 1 );
    return ;
}

void NumAdd ( uint8 *num ) {
    ++ ( *num );

    if ( ( *num ) >= 10 ) {
        *num = 0;
    }

    return ;
}

#define WATCH_DOG 1

int main ( void ) {
    WDT_CONTR = 0x3c;

    while ( 1 ) {
        static uint8 i = 0;
        SmgShow ( i );
        NumAdd ( &i );
#if WATCH_DOG
        WDT_CONTR = 0x3c; /* 喂狗 */
#endif
        delay500ms();
    }
}