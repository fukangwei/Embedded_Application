#include "STC12C5A.H"
#include "stdio.h"
#include "W5500.h"
#include <string.h>

typedef unsigned char uint8;
typedef unsigned int uint16;
typedef signed char int8;
typedef signed int int16;

sbit MAX485_DIR = P1 ^ 4; /* 设置MAX485信号的传输方向 */
sbit RL1 = P2 ^ 0;
sbit RL2 = P2 ^ 1;
sbit RL3 = P2 ^ 2;
sbit RL4 = P2 ^ 3;
sbit LED1 = P0 ^ 1; /* LED0 */

const char send_rs485[] = {0x04, 0x00, 0x01, 0x31, 0x00, 0x36}; /* 向rs485发送的数据 */

#define LOCK 0
#define UNLOCK !LOCK
#define PRINT_BIN 0 /* 如果需要打印二进制数据 */
#define SEND_MESSAGE "%d#%d#*" /* 定义发送的字符串格式 */

volatile uint8 lock = LOCK; /* 为串口1和串口2的同步操作设置锁 */
uint8 recv_data[24] = {0}; /* 接收数据缓冲区 */

void uart_first_init ( void ) {
    SCON = 0x50; /* 0101_0000，8位可变波特率，无奇偶校验位 */
    TMOD = 0x21; /* 0011_0001，设置定时器1为8位自动重装计数器 */
    TH1 = 0xFD; /* 设置定时器1自动重装数 */
    TL1 = 0xFD;
    TR1 = 1; /* 开定时器1 */
    ES = 1; /* 允许串口中断 */
    EA = 1; /* 开总中断 */
}

void uart_first_send_byte ( uint8 dat ) {
    TI = 0; /* 清零串口发送完成中断请求标志 */
    SBUF = dat;

    while ( TI == 0 ); /* 等待发送完成 */

    TI = 0; /* 清零串口发送完成中断请求标志 */
}

void uart_first_send_string ( uint8 *buf ) {
    while ( *buf != '\0' ) {
        uart_first_send_byte ( *buf++ );
    }
}

void uart_second_init ( void ) {
    AUXR &= 0xF7;
    S2CON = 0x50;
    AUXR |= 0x04;
    BRT = 0xDC;
    AUXR |= 0x10;
    IE2 = 0X01;
    EA = 1;
}

void uart_second_send_byte ( uint8 i ) {
    uint8 temp = 0;
    IE2 = 0X00; /* 关串口2中断，ES2 = 0 */
    S2CON = S2CON & 0XFD; /* 1111_1101，清零串口2发送完成中断请求标志 */
    S2BUF = i;

    do {
        temp = S2CON;
        temp = temp & 0x02;
    } while ( temp == 0 );

    S2CON = S2CON & 0XFD;  /* 1111_1101，清零串口2发送完成中断请求标志 */
    IE2 = 0X01; /* 允许串口2中断，ES2 = 1 */
}

void W5500_Initialization ( void ) {
    W5500_Init(); /* 初始化W5500寄存器函数 */
    Detect_Gateway_S0(); /* 检查网关服务器(socket 0) */
    Detect_Gateway_S1(); /* 检查网关服务器(socket 1) */
    Socket_Init ( 0 ); /* 指定Socket(0~7)初始化，初始化端口0 */
    Socket_Init ( 1 ); /* 指定Socket(0~7)初始化，初始化端口0 */
}

void Load_Net_Parameters ( void ) {
    Gateway_IP[0] = 192;
    Gateway_IP[1] = 168;
    Gateway_IP[2] = 1;
    Gateway_IP[3] = 1;
    Sub_Mask[0] = 255;
    Sub_Mask[1] = 255;
    Sub_Mask[2] = 255;
    Sub_Mask[3] = 0;
    Phy_Addr[0] = 0x0c;
    Phy_Addr[1] = 0x29;
    Phy_Addr[2] = 0xab;
    Phy_Addr[3] = 0x7c;
    Phy_Addr[4] = 0x00;
    Phy_Addr[5] = 0x01;
    IP_Addr[0] = 192;
    IP_Addr[1] = 168;
    IP_Addr[2] = 1;
    IP_Addr[3] = 199;
    S0_Port[0] = 0x13;
    S0_Port[1] = 0x88;
    UDP_DIPR[0] = 192; /* 加载端口0的目的IP地址 */
    UDP_DIPR[1] = 168;
    UDP_DIPR[2] = 1;
    UDP_DIPR[3] = 239;
    S0_Mode = UDP_MODE; /* 加载端口0的工作模式，即UDP模式 */
    S1_Port[0] = 0x13; /* 加载端口1的端口号5000 */
    S1_Port[1] = 0x88;
    S1_DIP[0] = 192; /* 加载端口1的目的IP地址 */
    S1_DIP[1] = 168;
    S1_DIP[2] = 1;
    S1_DIP[3] = 239;
    S1_DPort[0] = 0x13; /* 加载端口0的端口号5002 */
    S1_DPort[1] = 0x8A;
    S1_Mode = TCP_CLIENT; /* 加载端口1的工作模式，即TCP客户端模式 */
}

void W5500_Socket_Set ( void ) {
    if ( S0_State == 0 ) { /* 端口0初始化配置 */
        if ( S0_Mode == TCP_SERVER ) { /* TCP服务器模式 */
            if ( Socket_Listen ( 0 ) == TRUE ) {
                S0_State = S_INIT;
            } else {
                S0_State = 0;
            }
        } else if ( S0_Mode == TCP_CLIENT ) { /* TCP客户端模式 */
            if ( Socket_Connect ( 0 ) == TRUE ) {
                S0_State = S_INIT;
            } else {
                S0_State = 0;
            }
        } else { /* UDP模式 */
            if ( Socket_UDP ( 0 ) == TRUE ) {
                S0_State = S_INIT | S_CONN;
            } else {
                S0_State = 0;
            }
        }
    }

    if ( S1_State == 0 ) { /* 端口1初始化配置 */
        if ( S1_Mode == TCP_SERVER ) { /* TCP服务器模式 */
            if ( Socket_Listen ( 1 ) == TRUE ) {
                S1_State = S_INIT;
            } else {
                S1_State = 0;
            }
        } else if ( S1_Mode == TCP_CLIENT ) { /* TCP客户端模式 */
            if ( Socket_Connect ( 1 ) == TRUE ) {
                S1_State = S_INIT;
            } else {
                S1_State = 0;
            }
        }
    }
}

void delay250ms ( void ) {
    unsigned char a, b, c;

    for ( c = 74; c > 0; c-- )
        for ( b = 66; b > 0; b-- )
            for ( a = 140; a > 0; a-- );
}

void relay_init ( void ) {
    RL1 = 0;
    RL2 = 0;
    RL3 = 0;
    RL4 = 0;
}

void main ( void ) {
    relay_init(); /* 初始化继电器组 */
    uart_first_init(); /* 初始化串口1 */
    uart_second_init(); /* 初始化串口2 */
    Load_Net_Parameters(); /* 装载网络参数 */
    W5500_Hardware_Reset(); /* 硬件复位W5500 */
    W5500_Initialization(); /* W5500初始化配置 */
    delay250ms(); /* 延时操作 */

    while ( 1 ) {
        volatile int timecount = 5; /* 确定循环的次数 */
        int send_order = 0; /* 命令发送的次序 */
        int print_order = 0; /* 打印数据的次序 */
        W5500_Socket_Set(); /* W5500端口初始化配置 */
        W5500_Interrupt_Process(); /* W5500中断处理程序框架 */
        lock = LOCK; /* 首先要为数据发送操作上锁 */

        while ( ( lock != UNLOCK ) && ( timecount != 0 ) ) { /* 死循环，直到解锁或循环时间到 */
            MAX485_DIR = 1; /* 向MAX485发送命令 */

            for ( send_order = 0; send_order < sizeof ( send_rs485 ) / sizeof ( send_rs485[1] ); send_order++ ) {
                uart_second_send_byte ( send_rs485[send_order] );
            }

            MAX485_DIR = 0; /* 开始从MAX485接收数据 */
            timecount--; /* 循环次数减一 */
            delay250ms(); /* 延时操作 */
        }

        if ( lock == UNLOCK ) { /* 如果解锁成功，则打印数据，并进行处理 */
            char send_string[15] = {0}; /* 定义发送的字符串 */
            int cur_flow = 0; /* 定义当前流量 */
            int glo_flow = 0; /* 定义总流量 */
#if PRINT_BIN

            for ( print_order = 0; print_order < 24; print_order++ ) { /* 打印接收到的数据 */
                uart_first_send_byte ( recv_data[print_order] );
            }

#endif
            glo_flow = recv_data[10] * 10 + recv_data[11];
            cur_flow = recv_data[12] * 10 + recv_data[13];
            sprintf ( send_string, SEND_MESSAGE, cur_flow, glo_flow );
            uart_first_send_string ( send_string );
            /* --------------------- 使用W5500发送数据 ---------------------------*/
            if ( S0_State == ( S_INIT | S_CONN ) ) {
                uart_first_send_string ( "send" ); /* 调试功能“\^o^/” */
                S0_Data &= ~S_TRANSMITOK;
                /* 向主机的5000端口发数据 */
                UDP_DPORT[0] = 0x13;
                UDP_DPORT[1] = 0x88;
                Write_SOCK_Data_Buffer ( 0, send_string, strlen ( send_string ) ); /* 指定端口0发送数据 */
                /* 向主机的5001端口发数据 */
                UDP_DPORT[0] = 0x13;
                UDP_DPORT[1] = 0x89;
                Write_SOCK_Data_Buffer ( 0, send_string, strlen ( send_string ) ); /* 指定端口0发送数据 */
            }
        }

        if ( timecount == 0 ) { /* 如果循环时间到，则打印“0x66” */
            uart_first_send_byte ( 0x66 );
        }

        if ( ( S0_Data & S_RECEIVE ) == S_RECEIVE ) { /* 如果Socket0接收到数据 */
            unsigned char debug_string[20] = {0};
            unsigned char recv_string[15] = {0}; /* 从端口0接收命令字符串 */
            unsigned char command_string[15] = {0}; /* 命令字符串数据 */
            int result = 0; /* 解析结果 */
            int rl1_state, rl2_state, rl3_state; /* 继电器状态变量 */
            rl1_state = rl2_state = rl3_state = 0; /* 初始状态为0 */
            S0_Data &= ~S_RECEIVE;
            uart_first_send_string ( "I get it “\^o^/”" );
            Read_SOCK_Data_Buffer ( 0, recv_string );
            sprintf ( debug_string, "%x-%x", ( unsigned int ) recv_string[4], ( unsigned int ) recv_string[5] );
            uart_first_send_string ( debug_string );

            /*-------------------------------- 解析数据并控制继电器 ------------------------------*/
            if ( ( recv_string[4] == 0x13 ) && ( recv_string[5] == 0x88 ) ) { /* 判断对方是否为5000端口 */
                uart_first_send_string ( "It is sent by 5000!\r\n" );
            } else if ( ( recv_string[4] == 0x13 ) && ( recv_string[5] == 0x89 ) ) { /* 判断对方是否为5001端口 */
                uart_first_send_string ( "It is sent by 5001!\r\n" );
            }

            memcpy ( command_string, recv_string + 8, 5 ); /* 接收到的数据是从第八位开始的 */
            uart_first_send_string ( command_string ); /* 接收到的数据是从第八位开始的 */
            result = sscanf ( command_string, "%d,%d,%d", &rl1_state, &rl2_state, &rl3_state );

            if ( result == -1 ) { /* 如果解析失败 */
                uart_first_send_string ( "Oh, NO! Failure!\r\n" );
            }

            /* 输出接收到的数字 */
            uart_first_send_byte ( rl1_state + '0' );
            uart_first_send_byte ( rl2_state + '0' );
            uart_first_send_byte ( rl3_state + '0' );

            if ( rl1_state == 1 ) {
                RL1 = 1;
            } else if ( rl1_state == 0 ) {
                RL1 = 0;
            }

            if ( rl2_state == 1 ) {
                RL2 = 1;
            } else if ( rl2_state == 0 ) {
                RL2 = 0;
            }

            if ( rl3_state == 1 ) {
                RL3 = 1;
            } else if ( rl3_state == 0 ) {
                RL3 = 0;
            }

            if ( S1_State == ( S_INIT | S_CONN ) ) { /* 向目标端口发送“OK”字符串 */
                S1_Data &= ~S_TRANSMITOK;
                Write_SOCK_Data_Buffer ( 1, "OK\r\n", strlen ( "OK\r\n" ) );
            }
        }
    }
}

void uart_first_isr ( void ) interrupt 4 {
    volatile uint8 res = 0;

    if ( RI ) {
        RI = 0;
        res = SBUF;
        uart_first_send_byte ( res );
    } else {
        TI = 0;
    }
}

void uart_second_isr ( void ) interrupt 8 {
    volatile uint8 res = 0;
    static recv_true = 0; /* 判断接收数据的准确性，第一个数据为0x16则是正确的 */
    static uint8 recv_num = 0; /* 接收数据的次序 */
    lock = LOCK; /* 接收数据时需要上锁 */
    recv_true = 1; /* 默认情况下接收的数据是正确的 */
    recv_num = ( recv_num < 24 ) ? recv_num : 0; /* 判断是否小于24，否则设置为0 */
    res = S2CON;
    res = res & 0x01;
    LED1 = !LED1; /* 指示是否接收到数据 */

    if ( res == 1 ) {
        S2CON = S2CON & 0xFE; /* 1111_1110 */
        res = S2BUF;
        recv_data[recv_num++] = res; /* 将数据填入接收缓冲区中 */
    } else {
        S2CON = S2CON & 0xFD; /* 1111_1101 */
    }

    if ( recv_data[0] != 0x16 ) { /* 如果接收的第一个数据不是0x16，则将接收次序强制置为0 */
        recv_num = 0;
        recv_true = 0; /* 接收到的数据是错误的！ */
    } else if ( recv_data[0] == 0x16 ) {
        recv_true = 1; /* 接收到的数据是正确的！ */
    }

    if ( ( recv_num == 24 ) && ( recv_true == 1 ) ) { /* 接收到完整的数据，并且数据是正确的 */
        lock = UNLOCK; /* 数据接收完成时需要解锁 */
    }
}