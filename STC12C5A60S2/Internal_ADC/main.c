#include "STC12C5A.H"
#include <intrins.h>
#include "string.h"

#define uchar unsigned char
#define uint unsigned  int
typedef unsigned char  BYTE;
typedef unsigned int   WORD;

void UART_init ( void ) {
    PCON &= 0x7F;
    SCON = 0x50;
    AUXR &= 0xFB;
    BRT = 0xFD;
    AUXR |= 0x01;
    AUXR |= 0x10;
    ES = 1;
}

void UART_send_byte ( uchar dat ) {
    ES = 0;
    TI = 0;
    SBUF = dat;

    while ( TI == 0 );

    TI = 0;
    ES = 1;
}

void UART_send_string ( uchar *buf ) {
    while ( *buf != '\0' ) {
        UART_send_byte ( *buf++ );
    }
}

void Delay500ms ( void ) {
    unsigned char i, j, k;
    _nop_();
    _nop_();
    i = 22;
    j = 3;
    k = 227;

    do {
        do {
            while ( --k );
        } while ( --j );
    } while ( --i );
}

void InitADC ( void ) {
    P1ASF = 0xff; /* Set all P1 as Open-Drain mode */
    ADC_RES  = 0; /* Clear previous result */
    ADC_CONTR = ADC_POWER | ADC_SPEEDHH;
    AUXR1 = 0x00;
    Delay500ms();
}

WORD GetADCResult ( WORD ch ) {
    ADC_CONTR = ADC_POWER | ADC_SPEEDHH | ch | ADC_START;
    _nop_(); /* Must wait before inquiry */
    _nop_();
    _nop_();
    _nop_();

    while ( ! ( ADC_CONTR & ADC_FLAG ) ); /* Wait complete flag */

    ADC_CONTR &= ~ADC_FLAG; /* Close ADC */
    return ADC_RES; /* Return ADC result */
}

int main ( void ) {
    unsigned char tmp  = 0;
    unsigned char ge  = 0;
    unsigned char shi = 0;
    unsigned char bai = 0;
    unsigned char send_buf[4] = 0;
    memset ( send_buf, 0, sizeof ( send_buf ) );
    InitADC();
    UART_init();

    while ( 1 ) {
        tmp = GetADCResult ( 0 );
        ge  = tmp % 10;
        shi = ( tmp / 10 ) % 10;
        bai = tmp / 100;
        send_buf[0] = bai + '0';
        send_buf[1] = shi + '0';
        send_buf[2] = ge  + '0';
        UART_send_string ( send_buf );
        Delay500ms();
    }
}