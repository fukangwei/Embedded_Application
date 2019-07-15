#include "STC12C5A.H"
#include "string.h"
#include "nokia_5110.h"

#define uchar unsigned char
#define uint unsigned int

sbit left_wheel_1  = P2 ^ 0; /* 控制左轮的驱动板的端口1 */
sbit left_wheel_2  = P2 ^ 1; /* 控制左轮的驱动板的端口2 */
sbit right_wheel_1 = P2 ^ 2; /* 控制右轮的驱动板的端口1 */
sbit right_wheel_2 = P2 ^ 4; /* 控制右轮的驱动板的端口2 */

void car_forward ( void ); /* 小车前进 */
void car_stop ( void ); /* 小车停止 */
void car_back ( void ); /* 小车后退 */
void car_left ( void ); /* 小车左转 */
void car_right ( void ); /* 小车右转 */

void delay3s ( void ) { /* 单片机延时3秒钟 */
    unsigned char a, b, c, n;
    for ( c = 212; c > 0; c-- )
        for ( b = 162; b > 0; b-- )
            for ( a = 240; a > 0; a-- );
    for ( n = 5; n > 0; n-- );
}

void UART_init ( void ) { /* 对串口进行初始化 */
    TMOD = 0x20;
    SCON = 0x50;
    TH1 = 0xFD;
    TL1 = TH1;
    PCON = 0x00;
    EA = 1;
    ES = 1;
    TR1 = 1;
}

void UART_send_byte ( uchar dat ) { /* 串口发送单字节数据 */
    ES = 0;
    TI = 0;
    SBUF = dat;
    while ( TI == 0 );
    TI = 0;
    ES = 1;
}

void UART_send_string ( uchar* buf ) { /* 串口发送字符串数据 */
    while ( *buf != '\0' ) {
        UART_send_byte ( *buf++ );
    }
}

static char recv_buf[18] = {0}; /* 定义接收缓存区 */
static char show_buf[18] = {0}; /* 定义LCD5110显示缓存区 */
static char recv_count = 0; /* 定义串口接收数据的个数 */
static char recv_flag  = 0; /* 定义标志位 */
char* pChar = NULL; /* 定义字符指针 */

int main ( void ) {
    /* 对驱动板的端口进行初始化 */
    left_wheel_1  = 0;
    left_wheel_2  = 0;
    right_wheel_1 = 0;
    right_wheel_2 = 0;

    LCD_init(); /* LCD5110显示屏初始化 */
    UART_init(); /* 单片机串口初始化 */

    /* 对WiFi模块进行初始化 */
    delay3s();
    UART_send_string ( "AT+CWMODE=3\r\n" ); /* 设置为“softAP + station”共存模式 */
    delay3s();
    UART_send_string ( "AT+RST\r\n" ); /* 对设备进行重启 */
    delay3s();
    UART_send_string ( "AT+CIPMUX=1\r\n" ); /* “AT + CIPMUX = 1”时才能开启服务器 */
    delay3s();
    UART_send_string ( "AT+CIPSERVER=1,5000\r\n" ); /* 开启服务器模式，默认端口号为333 */
    delay3s();
    delay3s();

    recv_count = 0; /* 设置串口数据接收缓冲区下标 */

    while ( 1 ) {
        /* 如果数据处理标志位被启动，单片机进行数据处理操作 */
        if ( recv_flag == 1 ) {
            recv_count = 0;
            recv_flag = 0; /* 将数据处理标志位清零 */
            pChar = strstr ( recv_buf, "+IPD" ); /* 在串口接收数据缓冲区中寻找字符串“+PID” */
            if ( pChar != NULL ) {
                /* 把字符串“+PID”与其后面共10个字符拷贝进数据显示缓冲区中 */
                strncpy ( show_buf, pChar, 10 );

                switch ( show_buf[9] ) { /* 判断数据显示缓冲区的第9位 */
                    case 'p': /* 如果是“p”，则小车停止 */
                        car_stop();
                        break;
                    case 'f': /* 如果是“f”，则小车前进 */
                        car_forward();
                        break;
                    case 'l': /* 如果是“l”，则小车左转 */
                        car_left();
                        break;
                    case 'r': /* 如果是“r”，则小车右转 */
                        car_right();
                        break;
                    case 'b': /* 如果是“b”，则小车后退 */
                        car_back();
                        break;
                }

                memset ( show_buf, 0, sizeof ( show_buf ) ); /* 清空数据显示缓冲区 */
            }
        }
    }

    return 0;
}

void UARTInterrupt ( void ) interrupt 4 { /* 串口中断函数，接收WiFi模块的数据 */
    if ( RI ) {
        RI = 0;
        recv_buf[recv_count++] = SBUF; /* 接收WiFi模块的单字节数据 */

        /* 如果数据接收缓冲区已满，则重新接收WiFi模块数据，并启动数据处理标志位 */
        if ( recv_count == 18 ) {
            recv_count = 0;
            recv_flag = 1; /* 启动数据处理标志位 */
        }
    } else
        TI = 0;
}

void car_forward ( void ) { /* 小车前进 */
    /* 控制小车前进 */
    left_wheel_1  = 0;
    left_wheel_2  = 1;
    right_wheel_1 = 0;
    right_wheel_2 = 1;

    LCD_clear(); /* 对显示屏进行清屏 */
    LCD_write_english_string ( 0, 0, "forward" ); /* 在显示屏上显示字符串“forward” */
}

void car_stop ( void ) { /* 小车暂停 */
    /* 控制小车暂停 */
    left_wheel_1  = 0;
    left_wheel_2  = 0;
    right_wheel_1 = 0;
    right_wheel_2 = 0;

    LCD_clear(); /* 对显示屏进行清屏 */
    LCD_write_english_string ( 0, 0, "stop" ); /* 在显示屏上显示字符串“stop” */
}

void car_back ( void ) { /* 小车后退 */
    /* 控制小车后退 */
    left_wheel_1 = 0;
    left_wheel_2 = 1;
    right_wheel_1 = 0;
    right_wheel_2 = 1;

    LCD_clear(); /* 对显示屏进行清屏 */
    LCD_write_english_string ( 0, 0, "back" ); /* 在显示屏上显示字符串“back” */
}

void car_left ( void ) { /* 小车左转 */
    /* 控制小车左转 */
    left_wheel_1  = 0;
    left_wheel_2  = 1;
    right_wheel_1 = 0;
    right_wheel_2 = 1;

    LCD_clear(); /* 对显示屏进行清屏 */
    LCD_write_english_string ( 0, 0, "left" ); /* 在显示屏上显示字符串“left” */
}

void car_right ( void ) { /* 小车右转 */
    /* 控制小车右转 */
    left_wheel_1 = 0;
    left_wheel_2 = 1;
    right_wheel_1 = 0;
    right_wheel_2 = 1;

    LCD_clear(); /* 对显示屏进行清屏 */
    LCD_write_english_string ( 0, 0, "right" ); /* 在显示屏上显示字符串“right” */
}