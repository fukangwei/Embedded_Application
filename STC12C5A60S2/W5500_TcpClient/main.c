#include <reg51.h>
#include "W5500.h"
#include <string.h>
#include "intrins.h"

void Delay ( unsigned int d );

void W5500_Initialization ( void ) {
    W5500_Init();
    Detect_Gateway();
    Socket_Init ( 0 );
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
    IP_Addr[3] = 230;
    S0_Port[0] = 0x13;
    S0_Port[1] = 0x88;
    S0_DIP[0] = 192;
    S0_DIP[1] = 168;
    S0_DIP[2] = 1;
    S0_DIP[3] = 100;
    S0_DPort[0] = 0x17;
    S0_DPort[1] = 0x70;
    S0_Mode = TCP_CLIENT;
}

void W5500_Socket_Set ( void ) {
    if ( S0_State == 0 ) {
        if ( S0_Mode == TCP_SERVER ) {
            if ( Socket_Listen ( 0 ) == TRUE ) {
                S0_State = S_INIT;
            } else {
                S0_State = 0;
            }
        } else if ( S0_Mode == TCP_CLIENT ) {
            if ( Socket_Connect ( 0 ) == TRUE ) {
                S0_State = S_INIT;
            } else {
                S0_State = 0;
            }
        } else {
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
    Load_Net_Parameters();
    W5500_Hardware_Reset();
    W5500_Initialization();

    while ( 1 ) {
        W5500_Socket_Set();
        W5500_Interrupt_Process();

        if ( ( S0_Data & S_RECEIVE ) == S_RECEIVE ) {
            S0_Data &= ~S_RECEIVE;
            Process_Socket_Data ( 0 );
        } else if ( W5500_Send_Delay_Counter >= 3000 ) {
            if ( S0_State == ( S_INIT | S_CONN ) ) {
                S0_Data &= ~S_TRANSMITOK;
                memcpy ( Tx_Buffer, "\r\nWelcome To NiRenElec!\r\n", 23 );
                Write_SOCK_Data_Buffer ( 0, Tx_Buffer, 23 );
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