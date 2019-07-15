/* 数码管显示单片机串口接收的16进制数据 */
#include <reg52.h>

#define uchar unsigned char
#define uint  unsigned int

sbit DU = P2 ^ 6;
sbit WE = P2 ^ 7;

static uchar num_rec = 0;

uchar code table[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,
                      0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71};

void delayms ( uint ); /* 延时函数 */
void SmgShowHex ( uchar num ); /* 数码管显示16进制数据 */
void UART_init ( void ); /* 串口初始化函数 */

void main ( void ) {
    UART_init();
    while ( 1 ) {
        SmgShowHex ( num_rec );
    }
}

void delayms ( uint xms ) {
    uint i, j;
    for ( i = xms; i > 0; i-- )
        for ( j = 110; j > 0; j-- );
}

void SmgShowHex ( uchar num ) {
    uchar tmp = 0;

    /* 若数据大于0xf，则用两个数码管进行显示 */
    if ( num > 0xf ) {
        tmp = num / 16;
        DU = 1;
        P0 = table[tmp];
        DU = 0;
        P0 = 0xff;
        WE = 1;
        P0 = 0xef;
        WE = 0;
        delayms ( 1 );
    }

    tmp = num % 16;
    DU = 1;
    P0 = table[tmp];
    DU = 0;
    P0 = 0xff;
    WE = 1;
    P0 = 0xdf;
    WE = 0;
    delayms ( 1 );
}

void UART_init ( void ) {
    TMOD = 0x20;
    SCON = 0x50;
    TH1  = 0xFD;
    TL1  = TH1;
    PCON = 0x00;
    EA   = 1;
    ES   = 1;
    TR1  = 1;
}

void interrupt_uart() interrupt 4 {
    if ( RI ) {
        RI = 0;
        num_rec = SBUF; /* 将串口接收到的数据存入num_rec */
    } else
        TI = 0;
}