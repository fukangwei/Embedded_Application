#include <reg52.h>
#include "string.h"

typedef unsigned char uint8;
typedef unsigned int uint16;

sbit rotation = P1 ^ 0; /* 旋转编码器中的一只脚 */
sbit anotherbit = P1 ^ 2; /* 旋转编码器另一只脚 */
bit oldbit = 0; /* 上一状态暂存位 */

sbit DU  = P2 ^ 6;
sbit WE  = P2 ^ 7;
sbit LED = P1 ^ 7;

uint8 Buf[10] = {0};
unsigned char mycode[] = "0123456789";
uint8 code table[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x40};
signed int xuanzhuanzhi = 0; /* 旋转值 */
unsigned int TimeCount = 0; /* 定时计数器 */
unsigned char led1, led2, led3, ztj = 0;

void UART_send_byte ( uint8 dat ) {
    SBUF = dat;

    while ( TI == 0 );

    TI = 0;
}

void UART_send_string ( uint8 *buf ) {
    while ( *buf != '\0' ) {
        UART_send_byte ( *buf++ );
    }
}

void Init_Timer0 ( void ) { /* 10ms定时器 */
    TMOD = 0x01;
    TH0 = 0x0DC;
    TL0 = 0x00;
    EA = 1;
    ET0 = 1;
    TR0 = 1;
}

void UART_init ( void ) {
    SCON = 0x50;
    TMOD |= 0x20;
    TH1 = 0xFD;
    TL1 = 0xFD;
    TR1 = 1;
}

void ledscan ( void ) {
    switch ( ztj ) {
        case 0:
            DU = 1;
            P0 = table[led3];
            DU = 0;
            P0 = 0xff;
            WE = 1;
            P0 = 0xf7;
            WE = 0;
            ztj = 1;
            break;

        case 1:
            DU = 1;
            P0 = table[led2];
            DU = 0;
            P0 = 0xff;
            WE = 1;
            P0 = 0xfb;
            WE = 0;
            ztj = 2;
            break;

        case 2:
            DU = 1;
            P0 = table[led1];
            DU = 0;
            P0 = 0xff;
            WE = 1;
            P0 = 0xfd;
            WE = 0;
            ztj = 0;
            break;

        default:
            ztj = 0;
            break;
    }
}

int main ( void ) {
    memset ( Buf, 0 , sizeof ( Buf ) );
    Init_Timer0();
    UART_init();

    while ( 1 ) {
        if ( TimeCount <= 99 ) { /* 1s定时 */
            if ( oldbit == 1 && rotation == 0 ) { /* 判断前后状态以识别是否发生下降沿 */
                if ( anotherbit ) {
                    xuanzhuanzhi++; /* 为高是正转 */
                } else {
                    xuanzhuanzhi--; /* 为低是反转 */
                }
            }

            oldbit = rotation;
        } else {
            if ( xuanzhuanzhi < 0 ) {
                xuanzhuanzhi = -xuanzhuanzhi;
            }

            LED = !LED;
            xuanzhuanzhi = xuanzhuanzhi * 9 / 10; /* 等价于“xuanzhuanzhi * 360 / 400” */
            Buf[0] = mycode[ xuanzhuanzhi / 100 ] ;
            Buf[1] = mycode[ ( xuanzhuanzhi / 10 ) % 10 ];
            Buf[2] = mycode[ xuanzhuanzhi % 10 ];
            led1 = xuanzhuanzhi / 100;
            led2 = ( xuanzhuanzhi / 10 ) % 10;
            led3 = xuanzhuanzhi % 10;
            UART_send_string ( Buf );
            xuanzhuanzhi = 0;
            memset ( Buf, 0 , sizeof ( Buf ) );
            TimeCount = 0;
        }
    }
}

void Timer0_isr ( void ) interrupt 1 {
    TH0 = 0x0DC;
    TL0 = 0x00;
    ledscan();
    TimeCount++;
}