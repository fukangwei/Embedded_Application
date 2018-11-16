#include "W5500.h"
#include "intrins.h"

unsigned char Gateway_IP[4];
unsigned char Sub_Mask[4];
unsigned char Phy_Addr[6];
unsigned char IP_Addr[4];
unsigned char S0_Port[2];
unsigned char S0_DIP[4];
unsigned char S0_DPort[2];
unsigned char UDP_DIPR[4];
unsigned char UDP_DPORT[2];

unsigned char S1_DIP[4];
unsigned char S1_DPort[2];
unsigned char S1_Port[2];
unsigned char S1_Mode = 3;
unsigned char S1_State = 0;
unsigned char S1_Data;
unsigned char S0_Mode = 3;

#define TCP_SERVER 0x00
#define TCP_CLIENT 0x01
#define UDP_MODE   0x02

unsigned char S0_State = 0;
#define S_INIT 0x01
#define S_CONN 0x02

unsigned char S0_Data;
#define S_RECEIVE    0x01
#define S_TRANSMITOK 0x02

unsigned char Rx_Buffer[30];
unsigned char Tx_Buffer[30];
unsigned char W5500_Interrupt;

void Delay ( unsigned int  x ) {
    while ( x-- ) {
        unsigned char a, b;

        for ( b = 18; b > 0; b-- )
            for ( a = 152; a > 0; a-- );

        _nop_();
    }
}

unsigned char SPI_Read_Byte ( void ) {
    unsigned char i, rByte = 0;
    W5500_SCLK = 0;

    for ( i = 0; i < 8; i++ ) {
        W5500_SCLK = 1;
        rByte <<= 1;
        rByte |= W5500_MISO;
        W5500_SCLK = 0;
    }

    return rByte;
}

void SPI_Send_Byte ( unsigned char dt ) {
    unsigned char i;

    for ( i = 0; i < 8; i++ ) {
        W5500_SCLK = 0;

        if ( ( dt << i ) & 0x80 ) {
            W5500_MOSI = 1;
        } else {
            W5500_MOSI = 0;
        }

        W5500_SCLK = 1;
    }

    W5500_SCLK = 0;
}

void SPI_Send_Short ( unsigned short dt ) {
    SPI_Send_Byte ( ( unsigned char ) ( dt / 256 ) );
    SPI_Send_Byte ( dt );
}

void Write_W5500_1Byte ( unsigned short reg, unsigned char dat ) {
    W5500_SCS = 0;
    SPI_Send_Short ( reg );
    SPI_Send_Byte ( FDM1 | RWB_WRITE | COMMON_R );
    SPI_Send_Byte ( dat );
    W5500_SCS = 1;
}

void Write_W5500_2Byte ( unsigned short reg, unsigned short dat ) {
    W5500_SCS = 0;
    SPI_Send_Short ( reg );
    SPI_Send_Byte ( FDM2 | RWB_WRITE | COMMON_R );
    SPI_Send_Short ( dat );
    W5500_SCS = 1;
}

void Write_W5500_nByte ( unsigned short reg, unsigned char *dat_ptr, unsigned short size ) {
    unsigned short i;
    W5500_SCS = 0;
    SPI_Send_Short ( reg );
    SPI_Send_Byte ( VDM | RWB_WRITE | COMMON_R );

    for ( i = 0; i < size; i++ ) {
        SPI_Send_Byte ( *dat_ptr++ );
    }

    W5500_SCS = 1;
}

void Write_W5500_SOCK_1Byte ( SOCKET s, unsigned short reg, unsigned char dat ) {
    W5500_SCS = 0;
    SPI_Send_Short ( reg );
    SPI_Send_Byte ( FDM1 | RWB_WRITE | ( s * 0x20 + 0x08 ) );
    SPI_Send_Byte ( dat );
    W5500_SCS = 1;
}

void Write_W5500_SOCK_2Byte ( SOCKET s, unsigned short reg, unsigned short dat ) {
    W5500_SCS = 0;
    SPI_Send_Short ( reg );
    SPI_Send_Byte ( FDM2 | RWB_WRITE | ( s * 0x20 + 0x08 ) );
    SPI_Send_Short ( dat );
    W5500_SCS = 1;
}

void Write_W5500_SOCK_4Byte ( SOCKET s, unsigned short reg, unsigned char *dat_ptr ) {
    W5500_SCS = 0;
    SPI_Send_Short ( reg );
    SPI_Send_Byte ( FDM4 | RWB_WRITE | ( s * 0x20 + 0x08 ) );
    SPI_Send_Byte ( *dat_ptr++ );
    SPI_Send_Byte ( *dat_ptr++ );
    SPI_Send_Byte ( *dat_ptr++ );
    SPI_Send_Byte ( *dat_ptr++ );
    W5500_SCS = 1;
}

unsigned char Read_W5500_1Byte ( unsigned short reg ) {
    unsigned char i;
    W5500_SCS = 0;
    SPI_Send_Short ( reg );
    SPI_Send_Byte ( FDM1 | RWB_READ | COMMON_R );
    i = SPI_Read_Byte();
    W5500_SCS = 1;
    return i;
}

unsigned char Read_W5500_SOCK_1Byte ( SOCKET s, unsigned short reg ) {
    unsigned char i;
    W5500_SCS = 0;
    SPI_Send_Short ( reg );
    SPI_Send_Byte ( FDM1 | RWB_READ | ( s * 0x20 + 0x08 ) );
    i = SPI_Read_Byte();
    W5500_SCS = 1;
    return i;
}

unsigned short Read_W5500_SOCK_2Byte ( SOCKET s, unsigned short reg ) {
    unsigned short i;
    W5500_SCS = 0;
    SPI_Send_Short ( reg );
    SPI_Send_Byte ( FDM2 | RWB_READ | ( s * 0x20 + 0x08 ) );
    i = SPI_Read_Byte();
    i *= 256;
    i += SPI_Read_Byte();
    W5500_SCS = 1;
    return i;
}

unsigned short Read_SOCK_Data_Buffer ( SOCKET s, unsigned char *dat_ptr ) {
    unsigned short rx_size;
    unsigned short offset, offset1;
    unsigned short i;
    unsigned char j;
    rx_size = Read_W5500_SOCK_2Byte ( s, Sn_RX_RSR );

    if ( rx_size == 0 ) {
        return 0;
    }

    if ( rx_size > 1460 ) {
        rx_size = 1460;
    }

    offset = Read_W5500_SOCK_2Byte ( s, Sn_RX_RD );
    offset1 = offset;
    offset &= ( S_RX_SIZE - 1 );
    W5500_SCS = 0;
    SPI_Send_Short ( offset );
    SPI_Send_Byte ( VDM | RWB_READ | ( s * 0x20 + 0x18 ) );

    if ( ( offset + rx_size ) < S_RX_SIZE ) {
        for ( i = 0; i < rx_size; i++ ) {
            j = SPI_Read_Byte();
            *dat_ptr = j;
            dat_ptr++;
        }
    } else {
        offset = S_RX_SIZE - offset;

        for ( i = 0; i < offset; i++ ) {
            j = SPI_Read_Byte();
            *dat_ptr = j;
            dat_ptr++;
        }

        W5500_SCS = 1;
        W5500_SCS = 0;
        SPI_Send_Short ( 0x00 );
        SPI_Send_Byte ( VDM | RWB_READ | ( s * 0x20 + 0x18 ) );

        for ( ; i < rx_size; i++ ) {
            j = SPI_Read_Byte();
            *dat_ptr = j;
            dat_ptr++;
        }
    }

    W5500_SCS = 1;
    offset1 += rx_size;
    Write_W5500_SOCK_2Byte ( s, Sn_RX_RD, offset1 );
    Write_W5500_SOCK_1Byte ( s, Sn_CR, RECV );
    return rx_size;
}

void Write_SOCK_Data_Buffer ( SOCKET s, unsigned char *dat_ptr, unsigned short size ) {
    unsigned short offset, offset1;
    unsigned short i;

    if ( ( Read_W5500_SOCK_1Byte ( s, Sn_MR ) & 0x0f ) != SOCK_UDP ) {
        Write_W5500_SOCK_4Byte ( s, Sn_DIPR, UDP_DIPR );
        Write_W5500_SOCK_2Byte ( s, Sn_DPORTR, UDP_DPORT[0] * 256 + UDP_DPORT[1] );
    }

    offset = Read_W5500_SOCK_2Byte ( s, Sn_TX_WR );
    offset1 = offset;
    offset &= ( S_TX_SIZE - 1 );
    W5500_SCS = 0;
    SPI_Send_Short ( offset );
    SPI_Send_Byte ( VDM | RWB_WRITE | ( s * 0x20 + 0x10 ) );

    if ( ( offset + size ) < S_TX_SIZE ) {
        for ( i = 0; i < size; i++ ) {
            SPI_Send_Byte ( *dat_ptr++ );
        }
    } else {
        offset = S_TX_SIZE - offset;

        for ( i = 0; i < offset; i++ ) {
            SPI_Send_Byte ( *dat_ptr++ );
        }

        W5500_SCS = 1;
        W5500_SCS = 0;
        SPI_Send_Short ( 0x00 );
        SPI_Send_Byte ( VDM | RWB_WRITE | ( s * 0x20 + 0x10 ) );

        for ( ; i < size; i++ ) {
            SPI_Send_Byte ( *dat_ptr++ );
        }
    }

    W5500_SCS = 1;
    offset1 += size;
    Write_W5500_SOCK_2Byte ( s, Sn_TX_WR, offset1 );
    Write_W5500_SOCK_1Byte ( s, Sn_CR, SEND );
}

void W5500_Hardware_Reset ( void ) {
    W5500_RST = 0;
    Delay ( 200 );
    W5500_RST = 1;
    Delay ( 200 );

    while ( ( Read_W5500_1Byte ( PHYCFGR ) &LINK ) == 0 );
}

void W5500_Init ( void ) {
    unsigned char i = 0;
    Write_W5500_1Byte ( MR, RST );
    Delay ( 10 );
    Write_W5500_nByte ( GAR, Gateway_IP, 4 );
    Write_W5500_nByte ( SUBR, Sub_Mask, 4 );
    Write_W5500_nByte ( SHAR, Phy_Addr, 6 );
    Write_W5500_nByte ( SIPR, IP_Addr, 4 );

    for ( i = 0; i < 8; i++ ) {
        Write_W5500_SOCK_1Byte ( i, Sn_RXBUF_SIZE, 0x02 );
        Write_W5500_SOCK_1Byte ( i, Sn_TXBUF_SIZE, 0x02 );
    }

    Write_W5500_2Byte ( RTR, 0x07d0 );
    Write_W5500_1Byte ( RCR, 8 );
}

unsigned char Detect_Gateway_S0 ( void ) {
    unsigned char ip_adde[4];
    ip_adde[0] = IP_Addr[0] + 1;
    ip_adde[1] = IP_Addr[1] + 1;
    ip_adde[2] = IP_Addr[2] + 1;
    ip_adde[3] = IP_Addr[3] + 1;
    Write_W5500_SOCK_4Byte ( 0, Sn_DIPR, ip_adde );
    Write_W5500_SOCK_1Byte ( 0, Sn_MR, MR_TCP );
    Write_W5500_SOCK_1Byte ( 0, Sn_CR, OPEN );
    Delay ( 5 );

    if ( Read_W5500_SOCK_1Byte ( 0, Sn_SR ) != SOCK_INIT ) {
        Write_W5500_SOCK_1Byte ( 0, Sn_CR, CLOSE );
        return FALSE;
    }

    Write_W5500_SOCK_1Byte ( 0, Sn_CR, CONNECT );

    do {
        unsigned char j = 0;
        j = Read_W5500_SOCK_1Byte ( 0, Sn_IR );

        if ( j != 0 ) {
            Write_W5500_SOCK_1Byte ( 0, Sn_IR, j );
        }

        Delay ( 5 );

        if ( ( j & IR_TIMEOUT ) == IR_TIMEOUT ) {
            return FALSE;
        } else if ( Read_W5500_SOCK_1Byte ( 0, Sn_DHAR ) != 0xff ) {
            Write_W5500_SOCK_1Byte ( 0, Sn_CR, CLOSE );
            return TRUE;
        }
    } while ( 1 );
}

unsigned char Detect_Gateway_S1 ( void ) {
    unsigned char ip_adde[4];
    ip_adde[0] = IP_Addr[0] + 1;
    ip_adde[1] = IP_Addr[1] + 1;
    ip_adde[2] = IP_Addr[2] + 1;
    ip_adde[3] = IP_Addr[3] + 1;
    Write_W5500_SOCK_4Byte ( 1, Sn_DIPR, ip_adde );
    Write_W5500_SOCK_1Byte ( 1, Sn_MR, MR_TCP );
    Write_W5500_SOCK_1Byte ( 1, Sn_CR, OPEN );
    Delay ( 5 );

    if ( Read_W5500_SOCK_1Byte ( 1, Sn_SR ) != SOCK_INIT ) {
        Write_W5500_SOCK_1Byte ( 1, Sn_CR, CLOSE );
        return FALSE;
    }

    Write_W5500_SOCK_1Byte ( 1, Sn_CR, CONNECT );

    do {
        unsigned char j = 0;
        j = Read_W5500_SOCK_1Byte ( 1, Sn_IR );

        if ( j != 0 ) {
            Write_W5500_SOCK_1Byte ( 1, Sn_IR, j );
        }

        Delay ( 5 );

        if ( ( j & IR_TIMEOUT ) == IR_TIMEOUT ) {
            return FALSE;
        } else if ( Read_W5500_SOCK_1Byte ( 1, Sn_DHAR ) != 0xff ) {
            Write_W5500_SOCK_1Byte ( 1, Sn_CR, CLOSE );
            return TRUE;
        }
    } while ( 1 );
}

void Socket_Init ( SOCKET s ) {
    Write_W5500_SOCK_2Byte ( 0, Sn_MSSR, 30 );

    switch ( s ) {
        case 0:
            Write_W5500_SOCK_2Byte ( 0, Sn_PORT, S0_Port[0] * 256 + S0_Port[1] );
            break;

        case 1:
            Write_W5500_SOCK_2Byte ( 1, Sn_PORT, S1_Port[0] * 256 + S1_Port[1] );
            Write_W5500_SOCK_2Byte ( 1, Sn_DPORTR, S1_DPort[0] * 256 + S1_DPort[1] );
            Write_W5500_SOCK_4Byte ( 1, Sn_DIPR, S1_DIP );
            break;

        case 2:
            break;

        case 3:
            break;

        case 4:
            break;

        case 5:
            break;

        case 6:
            break;

        case 7:
            break;

        default:
            break;
    }
}

unsigned char Socket_Connect ( SOCKET s ) {
    Write_W5500_SOCK_1Byte ( s, Sn_MR, MR_TCP );
    Write_W5500_SOCK_1Byte ( s, Sn_CR, OPEN );
    Delay ( 5 );

    if ( Read_W5500_SOCK_1Byte ( s, Sn_SR ) != SOCK_INIT ) {
        Write_W5500_SOCK_1Byte ( s, Sn_CR, CLOSE );
        return FALSE;
    }

    Write_W5500_SOCK_1Byte ( s, Sn_CR, CONNECT );
    return TRUE;
}

unsigned char Socket_Listen ( SOCKET s ) {
    Write_W5500_SOCK_1Byte ( s, Sn_MR, MR_TCP );
    Write_W5500_SOCK_1Byte ( s, Sn_CR, OPEN );
    Delay ( 5 );

    if ( Read_W5500_SOCK_1Byte ( s, Sn_SR ) != SOCK_INIT ) {
        Write_W5500_SOCK_1Byte ( s, Sn_CR, CLOSE );
        return FALSE;
    }

    Write_W5500_SOCK_1Byte ( s, Sn_CR, LISTEN );
    Delay ( 5 );

    if ( Read_W5500_SOCK_1Byte ( s, Sn_SR ) != SOCK_LISTEN ) {
        Write_W5500_SOCK_1Byte ( s, Sn_CR, CLOSE );
        return FALSE;
    }

    return TRUE;
}

unsigned char Socket_UDP ( SOCKET s ) {
    Write_W5500_SOCK_1Byte ( s, Sn_MR, MR_UDP );
    Write_W5500_SOCK_1Byte ( s, Sn_CR, OPEN );
    Delay ( 5 );

    if ( Read_W5500_SOCK_1Byte ( s, Sn_SR ) != SOCK_UDP ) {
        Write_W5500_SOCK_1Byte ( s, Sn_CR, CLOSE );
        return FALSE;
    } else {
        return TRUE;
    }
}

void W5500_Interrupt_Process ( void ) {
    unsigned char i, j;
IntDispose:
    i = Read_W5500_1Byte ( SIR );

    if ( ( i & S0_INT ) == S0_INT ) {
        j = Read_W5500_SOCK_1Byte ( 0, Sn_IR );
        Write_W5500_SOCK_1Byte ( 0, Sn_IR, j );

        if ( j & IR_CON ) {
            S0_State |= S_CONN;
        }

        if ( j & IR_DISCON ) {
            Write_W5500_SOCK_1Byte ( 0, Sn_CR, CLOSE );
            Socket_Init ( 0 );
            S0_State = 0;
        }

        if ( j & IR_SEND_OK ) {
            S0_Data |= S_TRANSMITOK;
        }

        if ( j & IR_RECV ) {
            S0_Data |= S_RECEIVE;
        }

        if ( j & IR_TIMEOUT ) {
            Write_W5500_SOCK_1Byte ( 0, Sn_CR, CLOSE );
            S0_State = 0;
        }
    }

    if ( ( i & S1_INT ) == S1_INT ) {
        j = Read_W5500_SOCK_1Byte ( 1, Sn_IR );
        Write_W5500_SOCK_1Byte ( 1, Sn_IR, j );

        if ( j & IR_CON ) {
            S1_State |= S_CONN;
        }

        if ( j & IR_DISCON ) {
            Write_W5500_SOCK_1Byte ( 1, Sn_CR, CLOSE );
            Socket_Init ( 1 );
            S1_State = 0;
        }

        if ( j & IR_SEND_OK ) {
            S1_Data |= S_TRANSMITOK;
        }

        if ( j & IR_RECV ) {
            S1_Data |= S_RECEIVE;
        }

        if ( j & IR_TIMEOUT ) {
            Write_W5500_SOCK_1Byte ( 1, Sn_CR, CLOSE );
            S1_State = 0;
        }
    }

    if ( Read_W5500_1Byte ( SIR ) != 0 ) {
        goto IntDispose;
    }
}