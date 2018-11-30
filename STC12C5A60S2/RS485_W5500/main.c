#include "STC12C5A.H"
#include "stdio.h"
#include "W5500.h"
#include <string.h>

typedef unsigned char uint8;
typedef unsigned int uint16;
typedef signed char int8;
typedef signed int int16;

sbit MAX485_DIR = P1 ^ 4; /* ����MAX485�źŵĴ��䷽�� */
sbit RL1 = P2 ^ 0;
sbit RL2 = P2 ^ 1;
sbit RL3 = P2 ^ 2;
sbit RL4 = P2 ^ 3;
sbit LED1 = P0 ^ 1; /* LED0 */

const char send_rs485[] = {0x04, 0x00, 0x01, 0x31, 0x00, 0x36}; /* ��rs485���͵����� */

#define LOCK 0
#define UNLOCK !LOCK
#define PRINT_BIN 0 /* �����Ҫ��ӡ���������� */
#define SEND_MESSAGE "%d#%d#*" /* ���巢�͵��ַ�����ʽ */

volatile uint8 lock = LOCK; /* Ϊ����1�ʹ���2��ͬ������������ */
uint8 recv_data[24] = {0}; /* �������ݻ����� */

void uart_first_init ( void ) {
    SCON = 0x50; /* 0101_0000��8λ�ɱ䲨���ʣ�����żУ��λ */
    TMOD = 0x21; /* 0011_0001�����ö�ʱ��1Ϊ8λ�Զ���װ������ */
    TH1 = 0xFD; /* ���ö�ʱ��1�Զ���װ�� */
    TL1 = 0xFD;
    TR1 = 1; /* ����ʱ��1 */
    ES = 1; /* �������ж� */
    EA = 1; /* �����ж� */
}

void uart_first_send_byte ( uint8 dat ) {
    TI = 0; /* ���㴮�ڷ�������ж������־ */
    SBUF = dat;

    while ( TI == 0 ); /* �ȴ�������� */

    TI = 0; /* ���㴮�ڷ�������ж������־ */
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
    IE2 = 0X00; /* �ش���2�жϣ�ES2 = 0 */
    S2CON = S2CON & 0XFD; /* 1111_1101�����㴮��2��������ж������־ */
    S2BUF = i;

    do {
        temp = S2CON;
        temp = temp & 0x02;
    } while ( temp == 0 );

    S2CON = S2CON & 0XFD;  /* 1111_1101�����㴮��2��������ж������־ */
    IE2 = 0X01; /* ������2�жϣ�ES2 = 1 */
}

void W5500_Initialization ( void ) {
    W5500_Init(); /* ��ʼ��W5500�Ĵ������� */
    Detect_Gateway_S0(); /* ������ط�����(socket 0) */
    Detect_Gateway_S1(); /* ������ط�����(socket 1) */
    Socket_Init ( 0 ); /* ָ��Socket(0~7)��ʼ������ʼ���˿�0 */
    Socket_Init ( 1 ); /* ָ��Socket(0~7)��ʼ������ʼ���˿�0 */
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
    UDP_DIPR[0] = 192; /* ���ض˿�0��Ŀ��IP��ַ */
    UDP_DIPR[1] = 168;
    UDP_DIPR[2] = 1;
    UDP_DIPR[3] = 239;
    S0_Mode = UDP_MODE; /* ���ض˿�0�Ĺ���ģʽ����UDPģʽ */
    S1_Port[0] = 0x13; /* ���ض˿�1�Ķ˿ں�5000 */
    S1_Port[1] = 0x88;
    S1_DIP[0] = 192; /* ���ض˿�1��Ŀ��IP��ַ */
    S1_DIP[1] = 168;
    S1_DIP[2] = 1;
    S1_DIP[3] = 239;
    S1_DPort[0] = 0x13; /* ���ض˿�0�Ķ˿ں�5002 */
    S1_DPort[1] = 0x8A;
    S1_Mode = TCP_CLIENT; /* ���ض˿�1�Ĺ���ģʽ����TCP�ͻ���ģʽ */
}

void W5500_Socket_Set ( void ) {
    if ( S0_State == 0 ) { /* �˿�0��ʼ������ */
        if ( S0_Mode == TCP_SERVER ) { /* TCP������ģʽ */
            if ( Socket_Listen ( 0 ) == TRUE ) {
                S0_State = S_INIT;
            } else {
                S0_State = 0;
            }
        } else if ( S0_Mode == TCP_CLIENT ) { /* TCP�ͻ���ģʽ */
            if ( Socket_Connect ( 0 ) == TRUE ) {
                S0_State = S_INIT;
            } else {
                S0_State = 0;
            }
        } else { /* UDPģʽ */
            if ( Socket_UDP ( 0 ) == TRUE ) {
                S0_State = S_INIT | S_CONN;
            } else {
                S0_State = 0;
            }
        }
    }

    if ( S1_State == 0 ) { /* �˿�1��ʼ������ */
        if ( S1_Mode == TCP_SERVER ) { /* TCP������ģʽ */
            if ( Socket_Listen ( 1 ) == TRUE ) {
                S1_State = S_INIT;
            } else {
                S1_State = 0;
            }
        } else if ( S1_Mode == TCP_CLIENT ) { /* TCP�ͻ���ģʽ */
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
    relay_init(); /* ��ʼ���̵����� */
    uart_first_init(); /* ��ʼ������1 */
    uart_second_init(); /* ��ʼ������2 */
    Load_Net_Parameters(); /* װ��������� */
    W5500_Hardware_Reset(); /* Ӳ����λW5500 */
    W5500_Initialization(); /* W5500��ʼ������ */
    delay250ms(); /* ��ʱ���� */

    while ( 1 ) {
        volatile int timecount = 5; /* ȷ��ѭ���Ĵ��� */
        int send_order = 0; /* ����͵Ĵ��� */
        int print_order = 0; /* ��ӡ���ݵĴ��� */
        W5500_Socket_Set(); /* W5500�˿ڳ�ʼ������ */
        W5500_Interrupt_Process(); /* W5500�жϴ�������� */
        lock = LOCK; /* ����ҪΪ���ݷ��Ͳ������� */

        while ( ( lock != UNLOCK ) && ( timecount != 0 ) ) { /* ��ѭ����ֱ��������ѭ��ʱ�䵽 */
            MAX485_DIR = 1; /* ��MAX485�������� */

            for ( send_order = 0; send_order < sizeof ( send_rs485 ) / sizeof ( send_rs485[1] ); send_order++ ) {
                uart_second_send_byte ( send_rs485[send_order] );
            }

            MAX485_DIR = 0; /* ��ʼ��MAX485�������� */
            timecount--; /* ѭ��������һ */
            delay250ms(); /* ��ʱ���� */
        }

        if ( lock == UNLOCK ) { /* ��������ɹ������ӡ���ݣ������д��� */
            char send_string[15] = {0}; /* ���巢�͵��ַ��� */
            int cur_flow = 0; /* ���嵱ǰ���� */
            int glo_flow = 0; /* ���������� */
#if PRINT_BIN

            for ( print_order = 0; print_order < 24; print_order++ ) { /* ��ӡ���յ������� */
                uart_first_send_byte ( recv_data[print_order] );
            }

#endif
            glo_flow = recv_data[10] * 10 + recv_data[11];
            cur_flow = recv_data[12] * 10 + recv_data[13];
            sprintf ( send_string, SEND_MESSAGE, cur_flow, glo_flow );
            uart_first_send_string ( send_string );
            /* --------------------- ʹ��W5500�������� ---------------------------*/
            if ( S0_State == ( S_INIT | S_CONN ) ) {
                uart_first_send_string ( "send" ); /* ���Թ��ܡ�\^o^/�� */
                S0_Data &= ~S_TRANSMITOK;
                /* ��������5000�˿ڷ����� */
                UDP_DPORT[0] = 0x13;
                UDP_DPORT[1] = 0x88;
                Write_SOCK_Data_Buffer ( 0, send_string, strlen ( send_string ) ); /* ָ���˿�0�������� */
                /* ��������5001�˿ڷ����� */
                UDP_DPORT[0] = 0x13;
                UDP_DPORT[1] = 0x89;
                Write_SOCK_Data_Buffer ( 0, send_string, strlen ( send_string ) ); /* ָ���˿�0�������� */
            }
        }

        if ( timecount == 0 ) { /* ���ѭ��ʱ�䵽�����ӡ��0x66�� */
            uart_first_send_byte ( 0x66 );
        }

        if ( ( S0_Data & S_RECEIVE ) == S_RECEIVE ) { /* ���Socket0���յ����� */
            unsigned char debug_string[20] = {0};
            unsigned char recv_string[15] = {0}; /* �Ӷ˿�0���������ַ��� */
            unsigned char command_string[15] = {0}; /* �����ַ������� */
            int result = 0; /* ������� */
            int rl1_state, rl2_state, rl3_state; /* �̵���״̬���� */
            rl1_state = rl2_state = rl3_state = 0; /* ��ʼ״̬Ϊ0 */
            S0_Data &= ~S_RECEIVE;
            uart_first_send_string ( "I get it ��\^o^/��" );
            Read_SOCK_Data_Buffer ( 0, recv_string );
            sprintf ( debug_string, "%x-%x", ( unsigned int ) recv_string[4], ( unsigned int ) recv_string[5] );
            uart_first_send_string ( debug_string );

            /*-------------------------------- �������ݲ����Ƽ̵��� ------------------------------*/
            if ( ( recv_string[4] == 0x13 ) && ( recv_string[5] == 0x88 ) ) { /* �ж϶Է��Ƿ�Ϊ5000�˿� */
                uart_first_send_string ( "It is sent by 5000!\r\n" );
            } else if ( ( recv_string[4] == 0x13 ) && ( recv_string[5] == 0x89 ) ) { /* �ж϶Է��Ƿ�Ϊ5001�˿� */
                uart_first_send_string ( "It is sent by 5001!\r\n" );
            }

            memcpy ( command_string, recv_string + 8, 5 ); /* ���յ��������Ǵӵڰ�λ��ʼ�� */
            uart_first_send_string ( command_string ); /* ���յ��������Ǵӵڰ�λ��ʼ�� */
            result = sscanf ( command_string, "%d,%d,%d", &rl1_state, &rl2_state, &rl3_state );

            if ( result == -1 ) { /* �������ʧ�� */
                uart_first_send_string ( "Oh, NO! Failure!\r\n" );
            }

            /* ������յ������� */
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

            if ( S1_State == ( S_INIT | S_CONN ) ) { /* ��Ŀ��˿ڷ��͡�OK���ַ��� */
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
    static recv_true = 0; /* �жϽ������ݵ�׼ȷ�ԣ���һ������Ϊ0x16������ȷ�� */
    static uint8 recv_num = 0; /* �������ݵĴ��� */
    lock = LOCK; /* ��������ʱ��Ҫ���� */
    recv_true = 1; /* Ĭ������½��յ���������ȷ�� */
    recv_num = ( recv_num < 24 ) ? recv_num : 0; /* �ж��Ƿ�С��24����������Ϊ0 */
    res = S2CON;
    res = res & 0x01;
    LED1 = !LED1; /* ָʾ�Ƿ���յ����� */

    if ( res == 1 ) {
        S2CON = S2CON & 0xFE; /* 1111_1110 */
        res = S2BUF;
        recv_data[recv_num++] = res; /* ������������ջ������� */
    } else {
        S2CON = S2CON & 0xFD; /* 1111_1101 */
    }

    if ( recv_data[0] != 0x16 ) { /* ������յĵ�һ�����ݲ���0x16���򽫽��մ���ǿ����Ϊ0 */
        recv_num = 0;
        recv_true = 0; /* ���յ��������Ǵ���ģ� */
    } else if ( recv_data[0] == 0x16 ) {
        recv_true = 1; /* ���յ�����������ȷ�ģ� */
    }

    if ( ( recv_num == 24 ) && ( recv_true == 1 ) ) { /* ���յ����������ݣ�������������ȷ�� */
        lock = UNLOCK; /* ���ݽ������ʱ��Ҫ���� */
    }
}