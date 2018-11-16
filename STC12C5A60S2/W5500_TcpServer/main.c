#include <reg51.h>
#include "W5500.h"
#include <string.h>
#include "intrins.h"

void Delay ( unsigned int d );

void W5500_Initialization ( void ) {
    W5500_Init(); /* 初始化W5500寄存器函数 */
    Detect_Gateway(); /* 检查网关服务器 */
    Socket_Init ( 0 ); /* 指定Socket(0~7)初始化，初始化端口0 */
}

void Load_Net_Parameters ( void ) {
    Gateway_IP[0] = 192; /* 加载网关参数 */
    Gateway_IP[1] = 168;
    Gateway_IP[2] = 1;
    Gateway_IP[3] = 1;
    Sub_Mask[0] = 255; /* 加载子网掩码 */
    Sub_Mask[1] = 255;
    Sub_Mask[2] = 255;
    Sub_Mask[3] = 0;
    Phy_Addr[0] = 0x0c; /* 加载物理地址 */
    Phy_Addr[1] = 0x29;
    Phy_Addr[2] = 0xab;
    Phy_Addr[3] = 0x7c;
    Phy_Addr[4] = 0x00;
    Phy_Addr[5] = 0x01;
    IP_Addr[0] = 192; /* 加载本机IP地址 */
    IP_Addr[1] = 168;
    IP_Addr[2] = 1;
    IP_Addr[3] = 199;
    S0_Port[0] = 0x13; /* 加载端口0的端口号 */
    S0_Port[1] = 0x88;
    S0_Mode = TCP_SERVER; /* 加载端口0的工作模式(TCP服务端模式) */
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
}

void Process_Socket_Data ( SOCKET s ) {
    unsigned short size;
    size = Read_SOCK_Data_Buffer ( s, Rx_Buffer );
    memcpy ( Tx_Buffer, Rx_Buffer, size );
    Write_SOCK_Data_Buffer ( s, Tx_Buffer, size );
}

int main ( void ) {
    unsigned int W5500_Send_Delay_Counter = 0;
    Load_Net_Parameters(); /* 装载网络参数 */
    W5500_Hardware_Reset(); /* 硬件复位W5500 */
    W5500_Initialization(); /* W5500初始化配置 */

    while ( 1 ) {
        W5500_Socket_Set(); /* W5500端口初始化配置 */
        W5500_Interrupt_Process(); /* W5500中断处理程序框架 */

        if ( ( S0_Data & S_RECEIVE ) == S_RECEIVE ) { /* 如果Socket0接收到数据 */
            S0_Data &= ~S_RECEIVE;
            Process_Socket_Data ( 0 );
        } else if ( W5500_Send_Delay_Counter >= 3000 ) { /* 定时发送字符串 */
            if ( S0_State == ( S_INIT | S_CONN ) ) {
                S0_Data &= ~S_TRANSMITOK;
                memcpy ( Tx_Buffer, "\r\nWelcome To NiRenElec!\r\n", 23 );
                Write_SOCK_Data_Buffer ( 0, Tx_Buffer, 23 ); /* 指定Socket(0~7)发送数据处理，端口0发送23字节数据 */
            }

            W5500_Send_Delay_Counter = 0;
        }

        W5500_Send_Delay_Counter++;
    }
}

void Delay ( unsigned int  x ) {
    while ( x-- ) {
        unsigned char a, b;

        for ( b = 18; b > 0; b-- )
            for ( a = 152; a > 0; a-- );

        _nop_();
    }
}