#include "W5500.h"

unsigned char Gateway_IP[4]; /* ����IP��ַ */
unsigned char Sub_Mask[4];   /* �������� */
unsigned char Phy_Addr[6];   /* �����ַ(MAC) */
unsigned char IP_Addr[4];    /* ����IP��ַ */
unsigned char S0_Port[2];    /* �˿�0�Ķ˿ں� */
unsigned char S0_DIP[4];     /* �˿�0Ŀ��IP��ַ */
unsigned char S0_DPort[2];   /* �˿�0Ŀ�Ķ˿ں� */
unsigned char UDP_DIPR[4];   /* UDP(�㲥)ģʽ��Ŀ������IP��ַ */
unsigned char UDP_DPORT[2];  /* UDP(�㲥)ģʽ��Ŀ�������˿ں� */

unsigned char S0_Mode = 3;  /* �˿�0������ģʽ */

#define TCP_SERVER 0x00 /* TCP������ģʽ */
#define TCP_CLIENT 0x01 /* TCP�ͻ���ģʽ */
#define UDP_MODE   0x02 /* UDP(�㲥)ģʽ */

unsigned char S0_State = 0; /* �˿�0״̬��¼ */
#define S_INIT 0x01 /* �˿���ɳ�ʼ�� */
#define S_CONN 0x02 /* �˿�������ӣ����������������� */

unsigned char S0_Data; /* �˿�0���պͷ������ݵ�״̬ */
#define S_RECEIVE    0x01 /* �˿ڽ��յ�һ�����ݰ� */
#define S_TRANSMITOK 0x02 /* �˿ڷ���һ�����ݰ���� */

unsigned char Rx_Buffer[30]; /* �˿ڽ������ݻ����� */
unsigned char Tx_Buffer[30]; /* �˿ڷ������ݻ����� */
unsigned char W5500_Interrupt; /* W5500�жϱ�־(0�����жϣ�1�����ж�) */

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
    SPI_Send_Byte ( ( unsigned char ) ( dt / 256 ) ); /* д���ݸ�λ */
    SPI_Send_Byte ( dt ); /* д���ݵ�λ */
}

void Write_W5500_1Byte ( unsigned short reg, unsigned char dat ) {
    W5500_SCS = 0; /* ��W5500��SCSΪ�͵�ƽ */
    SPI_Send_Short ( reg ); /* ͨ��SPIд16λ�Ĵ�����ַ */
    SPI_Send_Byte ( FDM1 | RWB_WRITE | COMMON_R ); /* ͨ��SPIд�����ֽڣ�1���ֽ����ݳ��ȣ�д���ݣ�ѡ��ͨ�üĴ��� */
    SPI_Send_Byte ( dat ); /* д1���ֽ����� */
    W5500_SCS = 1; /* ��W5500��SCSΪ�ߵ�ƽ */
}

void Write_W5500_2Byte ( unsigned short reg, unsigned short dat ) {
    W5500_SCS = 0; /* ��W5500��SCSΪ�͵�ƽ */
    SPI_Send_Short ( reg ); /* ͨ��SPIд16λ�Ĵ�����ַ */
    SPI_Send_Byte ( FDM2 | RWB_WRITE | COMMON_R ); /* ͨ��SPIд�����ֽڣ�2���ֽ����ݳ��ȣ�д���ݣ�ѡ��ͨ�üĴ��� */
    SPI_Send_Short ( dat ); /* д16λ���� */
    W5500_SCS = 1; /* ��W5500��SCSΪ�ߵ�ƽ */
}

void Write_W5500_nByte ( unsigned short reg, unsigned char *dat_ptr, unsigned short size ) {
    unsigned short i;
    W5500_SCS = 0; /* ��W5500��SCSΪ�͵�ƽ */
    SPI_Send_Short ( reg ); /* ͨ��SPIд16λ�Ĵ�����ַ */
    SPI_Send_Byte ( VDM | RWB_WRITE | COMMON_R ); /* ͨ��SPIд�����ֽڣ�N���ֽ����ݳ��ȣ�д���ݣ�ѡ��ͨ�üĴ��� */

    for ( i = 0; i < size; i++ ) { /* ѭ������������size���ֽ�����д��W5500 */
        SPI_Send_Byte ( *dat_ptr++ ); /* дһ���ֽ����� */
    }

    W5500_SCS = 1; /* ��W5500��SCSΪ�ߵ�ƽ */
}

void Write_W5500_SOCK_1Byte ( SOCKET s, unsigned short reg, unsigned char dat ) {
    W5500_SCS = 0; /* ��W5500��SCSΪ�͵�ƽ */
    SPI_Send_Short ( reg ); /* ͨ��SPIд16λ�Ĵ�����ַ */
    SPI_Send_Byte ( FDM1 | RWB_WRITE | ( s * 0x20 + 0x08 ) ); /* ͨ��SPIд�����ֽڣ�1���ֽ����ݳ��ȣ�д���ݣ�ѡ��˿�s�ļĴ��� */
    SPI_Send_Byte ( dat ); /* д1���ֽ����� */
    W5500_SCS = 1; /* ��W5500��SCSΪ�ߵ�ƽ */
}

/* ͨ��SPI��ָ���˿ڼĴ���д2���ֽ����ݡ�����s�Ƕ˿ںţ�reg��16λ�Ĵ�����ַ��dat��16λ��д�������(2���ֽ�) */
void Write_W5500_SOCK_2Byte ( SOCKET s, unsigned short reg, unsigned short dat ) {
    W5500_SCS = 0; /* ��W5500��SCSΪ�͵�ƽ */
    SPI_Send_Short ( reg ); /* ͨ��SPIд16λ�Ĵ�����ַ */
    SPI_Send_Byte ( FDM2 | RWB_WRITE | ( s * 0x20 + 0x08 ) ); /* ͨ��SPIд�����ֽڣ�2���ֽ����ݳ��ȣ�д���ݣ�ѡ��˿�s�ļĴ��� */
    SPI_Send_Short ( dat ); /* д16λ���� */
    W5500_SCS = 1; /* ��W5500��SCSΪ�ߵ�ƽ */
}

/* ͨ��SPI��ָ���˿ڼĴ���д4���ֽ����ݡ�����s�Ƕ˿ںţ�reg��16λ�Ĵ�����ַ��dat_ptr�Ǵ�д���4���ֽڻ�����ָ�� */
void Write_W5500_SOCK_4Byte ( SOCKET s, unsigned short reg, unsigned char *dat_ptr ) {
    W5500_SCS = 0; /* ��W5500��SCSΪ�͵�ƽ */
    SPI_Send_Short ( reg ); /* ͨ��SPIд16λ�Ĵ�����ַ */
    SPI_Send_Byte ( FDM4 | RWB_WRITE | ( s * 0x20 + 0x08 ) ); /* ͨ��SPIд�����ֽڣ�4���ֽ����ݳ��ȣ�д���ݣ�ѡ��˿�s�ļĴ��� */
    SPI_Send_Byte ( *dat_ptr++ ); /* д��1���ֽ����� */
    SPI_Send_Byte ( *dat_ptr++ ); /* д��2���ֽ����� */
    SPI_Send_Byte ( *dat_ptr++ ); /* д��3���ֽ����� */
    SPI_Send_Byte ( *dat_ptr++ ); /* д��4���ֽ����� */
    W5500_SCS = 1; /* ��W5500��SCSΪ�ߵ�ƽ */
}

unsigned char Read_W5500_1Byte ( unsigned short reg ) { /* ��W5500ָ����ַ�Ĵ�����1���ֽ����ݡ�����reg��16λ�Ĵ�����ַ */
    unsigned char i;
    W5500_SCS = 0; /* ��W5500��SCSΪ�͵�ƽ */
    SPI_Send_Short ( reg ); /* ͨ��SPIд16λ�Ĵ�����ַ */
    SPI_Send_Byte ( FDM1 | RWB_READ | COMMON_R ); /* ͨ��SPIд�����ֽڣ�1���ֽ����ݳ��ȣ������ݣ�ѡ��ͨ�üĴ��� */
    i = SPI_Read_Byte();
    W5500_SCS = 1; /* ��W5500��SCSΪ�ߵ�ƽ */
    return i; /* ���ض�ȡ���ļĴ������� */
}

/* ��W5500ָ���˿ڼĴ�����1���ֽ����ݡ�����s�Ƕ˿ںţ�reg��16λ�Ĵ�����ַ */
unsigned char Read_W5500_SOCK_1Byte ( SOCKET s, unsigned short reg ) {
    unsigned char i;
    W5500_SCS = 0; /* ��W5500��SCSΪ�͵�ƽ */
    SPI_Send_Short ( reg ); /* ͨ��SPIд16λ�Ĵ�����ַ */
    SPI_Send_Byte ( FDM1 | RWB_READ | ( s * 0x20 + 0x08 ) ); /* ͨ��SPIд�����ֽڣ�1���ֽ����ݳ��ȣ������ݣ�ѡ��˿�s�ļĴ��� */
    i = SPI_Read_Byte();
    W5500_SCS = 1; /* ��W5500��SCSΪ�ߵ�ƽ */
    return i; /* ���ض�ȡ���ļĴ������� */
}

/* ��W5500ָ���˿ڼĴ�����2���ֽ����ݡ�����s�Ƕ˿ںţ�reg��16λ�Ĵ�����ַ */
unsigned short Read_W5500_SOCK_2Byte ( SOCKET s, unsigned short reg ) {
    unsigned short i;
    W5500_SCS = 0; /* ��W5500��SCSΪ�͵�ƽ */
    SPI_Send_Short ( reg ); /* ͨ��SPIд16λ�Ĵ�����ַ */
    SPI_Send_Byte ( FDM2 | RWB_READ | ( s * 0x20 + 0x08 ) ); /* ͨ��SPIд�����ֽڣ�2���ֽ����ݳ��ȣ������ݣ�ѡ��˿�s�ļĴ��� */
    i = SPI_Read_Byte();
    i *= 256;
    i += SPI_Read_Byte(); /* ��ȡ��λ���� */
    W5500_SCS = 1; /* ��W5500��SCSΪ�ߵ�ƽ */
    return i; /* ���ض�ȡ���ļĴ������� */
}

/* ��W5500�������ݻ������ж�ȡ���ݡ�����s�Ƕ˿ںţ�dat_ptr�����ݱ��滺����ָ�� */
unsigned short Read_SOCK_Data_Buffer ( SOCKET s, unsigned char *dat_ptr ) {
    unsigned short rx_size;
    unsigned short offset, offset1;
    unsigned short i;
    unsigned char j;
    rx_size = Read_W5500_SOCK_2Byte ( s, Sn_RX_RSR );

    if ( rx_size == 0 ) {
        return 0; /* û���յ������򷵻� */
    }

    if ( rx_size > 1460 ) {
        rx_size = 1460;
    }

    offset = Read_W5500_SOCK_2Byte ( s, Sn_RX_RD );
    offset1 = offset;
    offset &= ( S_RX_SIZE - 1 ); /* ����ʵ�ʵ������ַ */
    W5500_SCS = 0; /* ��W5500��SCSΪ�͵�ƽ */
    SPI_Send_Short ( offset ); /* д16λ��ַ */
    SPI_Send_Byte ( VDM | RWB_READ | ( s * 0x20 + 0x18 ) ); /* д�����ֽڣ�N���ֽ����ݳ��ȣ������ݣ�ѡ��˿�s�ļĴ��� */

    if ( ( offset + rx_size ) < S_RX_SIZE ) { /* �������ַδ����W5500���ջ������Ĵ���������ַ */
        for ( i = 0; i < rx_size; i++ ) { /* ѭ����ȡrx_size���ֽ����� */
            j = SPI_Read_Byte(); /* ��ȡ1���ֽ����� */
            *dat_ptr = j; /* ����ȡ�������ݱ��浽���ݱ��滺���� */
            dat_ptr++; /* ���ݱ��滺����ָ���ַ����1 */
        }
    } else { /* �������ַ����W5500���ջ������Ĵ���������ַ */
        offset = S_RX_SIZE - offset;

        for ( i = 0; i < offset; i++ ) { /* ѭ����ȡ��ǰoffset���ֽ����� */
            j = SPI_Read_Byte(); /* ��ȡ1���ֽ����� */
            *dat_ptr = j; /* ����ȡ�������ݱ��浽���ݱ��滺���� */
            dat_ptr++; /* ���ݱ��滺����ָ���ַ����1 */
        }

        W5500_SCS = 1; /* ��W5500��SCSΪ�ߵ�ƽ */
        W5500_SCS = 0; /* ��W5500��SCSΪ�͵�ƽ */
        SPI_Send_Short ( 0x00 ); /* д16λ��ַ */
        SPI_Send_Byte ( VDM | RWB_READ | ( s * 0x20 + 0x18 ) ); /* д�����ֽڣ�N���ֽ����ݳ��ȣ������ݣ�ѡ��˿�s�ļĴ��� */

        for ( ; i < rx_size; i++ ) { /* ѭ����ȡ��rx_size - offset�����ֽ����� */
            j = SPI_Read_Byte(); /* ��ȡ1���ֽ����� */
            *dat_ptr = j; /* ����ȡ�������ݱ��浽���ݱ��滺���� */
            dat_ptr++; /* ���ݱ��滺����ָ���ַ����1 */
        }
    }

    W5500_SCS = 1; /* ��W5500��SCSΪ�ߵ�ƽ */
    offset1 += rx_size; /* ����ʵ�������ַ�����´ζ�ȡ���յ������ݵ���ʼ��ַ */
    Write_W5500_SOCK_2Byte ( s, Sn_RX_RD, offset1 );
    Write_W5500_SOCK_1Byte ( s, Sn_CR, RECV ); /* ���������������� */
    return rx_size; /* ���ؽ��յ����ݵĳ��� */
}

/* ������д��W5500�����ݷ��ͻ�����������s�Ƕ˿ںţ�dat_ptr�����ݱ��滺����ָ�룬size�Ǵ�д�����ݵĳ��� */
void Write_SOCK_Data_Buffer ( SOCKET s, unsigned char *dat_ptr, unsigned short size ) {
    unsigned short offset, offset1;
    unsigned short i;

    /* �����UDPģʽ�������ڴ�����Ŀ��������IP�Ͷ˿ں� */
    if ( ( Read_W5500_SOCK_1Byte ( s, Sn_MR ) & 0x0f ) != SOCK_UDP ) { /* ���Socket��ʧ�� */
        Write_W5500_SOCK_4Byte ( s, Sn_DIPR, UDP_DIPR ); /* ����Ŀ������IP */
        Write_W5500_SOCK_2Byte ( s, Sn_DPORTR, UDP_DPORT[0] * 256 + UDP_DPORT[1] ); /* ����Ŀ�������˿ں� */
    }

    offset = Read_W5500_SOCK_2Byte ( s, Sn_TX_WR );
    offset1 = offset;
    offset &= ( S_TX_SIZE - 1 ); /* ����ʵ�ʵ������ַ */
    W5500_SCS = 0; /* ��W5500��SCSΪ�͵�ƽ */
    SPI_Send_Short ( offset ); /* д16λ��ַ */
    SPI_Send_Byte ( VDM | RWB_WRITE | ( s * 0x20 + 0x10 ) ); /* д�����ֽڣ�N���ֽ����ݳ��ȣ�д���ݣ�ѡ��˿�s�ļĴ��� */

    if ( ( offset + size ) < S_TX_SIZE ) { /* �������ַδ����W5500���ͻ������Ĵ���������ַ */
        for ( i = 0; i < size; i++ ) { /* ѭ��д��size���ֽ����� */
            SPI_Send_Byte ( *dat_ptr++ ); /* д��һ���ֽڵ����� */
        }
    } else { /* �������ַ����W5500���ͻ������Ĵ���������ַ */
        offset = S_TX_SIZE - offset;

        for ( i = 0; i < offset; i++ ) { /* ѭ��д��ǰoffset���ֽ����� */
            SPI_Send_Byte ( *dat_ptr++ ); /* д��һ���ֽڵ����� */
        }

        W5500_SCS = 1; /* ��W5500��SCSΪ�ߵ�ƽ */
        W5500_SCS = 0; /* ��W5500��SCSΪ�͵�ƽ */
        SPI_Send_Short ( 0x00 ); /* д16λ��ַ */
        SPI_Send_Byte ( VDM | RWB_WRITE | ( s * 0x20 + 0x10 ) ); /* д�����ֽڣ�N���ֽ����ݳ��ȣ�д���ݣ�ѡ��˿�s�ļĴ��� */

        for ( ; i < size; i++ ) { /* ѭ��д�롰size - offset�����ֽ����� */
            SPI_Send_Byte ( *dat_ptr++ ); /* д��һ���ֽڵ����� */
        }
    }

    W5500_SCS = 1; /* ��W5500��SCSΪ�ߵ�ƽ */
    offset1 += size; /* ����ʵ�������ַ���´�д���������ݵ��������ݻ���������ʼ��ַ */
    Write_W5500_SOCK_2Byte ( s, Sn_TX_WR, offset1 );
    Write_W5500_SOCK_1Byte ( s, Sn_CR, SEND ); /* ���������������� */
}

void W5500_Hardware_Reset ( void ) {
    W5500_RST = 0; /* ��λ�������� */
    Delay ( 200 );
    W5500_RST = 1; /* ��λ�������� */
    Delay ( 200 );

    while ( ( Read_W5500_1Byte ( PHYCFGR ) &LINK ) == 0 ); /* �ȴ���̫��������� */
}

void W5500_Init ( void ) {
    unsigned char i = 0;
    Write_W5500_1Byte ( MR, RST ); /* �����λW5500����1��Ч����λ���Զ���0 */
    Delay ( 10 ); /* ��ʱ10ms */
    Write_W5500_nByte ( GAR, Gateway_IP, 4 );
    Write_W5500_nByte ( SUBR, Sub_Mask, 4 );
    Write_W5500_nByte ( SHAR, Phy_Addr, 6 );
    Write_W5500_nByte ( SIPR, IP_Addr, 4 );

    for ( i = 0; i < 8; i++ ) { /* ���÷��ͻ������ͽ��ջ������Ĵ�С */
        Write_W5500_SOCK_1Byte ( i, Sn_RXBUF_SIZE, 0x02 ); /* Socket Rx memory size = 2k */
        Write_W5500_SOCK_1Byte ( i, Sn_TXBUF_SIZE, 0x02 ); /* Socket Tx mempry size = 2k */
    }

    Write_W5500_2Byte ( RTR, 0x07d0 );
    Write_W5500_1Byte ( RCR, 8 );
}

unsigned char Detect_Gateway ( void ) { /* ������ط��������ɹ�����TRUE(0xFF)��ʧ�ܷ���FALSE(0x00) */
    unsigned char ip_adde[4];
    ip_adde[0] = IP_Addr[0] + 1;
    ip_adde[1] = IP_Addr[1] + 1;
    ip_adde[2] = IP_Addr[2] + 1;
    ip_adde[3] = IP_Addr[3] + 1;
    /* ������ؼ���ȡ���ص������ַ */
    Write_W5500_SOCK_4Byte ( 0, Sn_DIPR, ip_adde ); /* ��Ŀ�ĵ�ַ�Ĵ���д���뱾��IP��ͬ��IPֵ */
    Write_W5500_SOCK_1Byte ( 0, Sn_MR, MR_TCP ); /* ����socketΪTCPģʽ */
    Write_W5500_SOCK_1Byte ( 0, Sn_CR, OPEN ); /* ��Socket */
    Delay ( 5 ); /* ��ʱ5ms */

    if ( Read_W5500_SOCK_1Byte ( 0, Sn_SR ) != SOCK_INIT ) { /* ���socket��ʧ�� */
        Write_W5500_SOCK_1Byte ( 0, Sn_CR, CLOSE ); /* �򿪲��ɹ����ر�Socket */
        return FALSE; /* ����FALSE(0x00) */
    }

    Write_W5500_SOCK_1Byte ( 0, Sn_CR, CONNECT ); /* ����SocketΪConnectģʽ */

    do {
        unsigned char j = 0;
        j = Read_W5500_SOCK_1Byte ( 0, Sn_IR ); /* ��ȡSocket0�жϱ�־�Ĵ��� */

        if ( j != 0 ) {
            Write_W5500_SOCK_1Byte ( 0, Sn_IR, j );
        }

        Delay ( 5 ); /* ��ʱ5ms */

        if ( ( j & IR_TIMEOUT ) == IR_TIMEOUT ) {
            return FALSE;
        } else if ( Read_W5500_SOCK_1Byte ( 0, Sn_DHAR ) != 0xff ) {
            Write_W5500_SOCK_1Byte ( 0, Sn_CR, CLOSE ); /* �ر�Socket */
            return TRUE;
        }
    } while ( 1 );
}

void Socket_Init ( SOCKET s ) { /* ָ��Socket(0~7)��ʼ��������s�Ǵ���ʼ���Ķ˿� */
    Write_W5500_SOCK_2Byte ( 0, Sn_MSSR, 30 ); /* ����Ƭ�ֽ��� */

    switch ( s ) {
        case 0:
            Write_W5500_SOCK_2Byte ( 0, Sn_PORT, S0_Port[0] * 256 + S0_Port[1] );
            break;

        case 1:
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

/* ����ָ��Socket(0~7)Ϊ�ͻ�����Զ�̷��������ӣ�����s�Ǵ��趨�Ķ˿ڡ��ɹ�����TRUE(0xFF)��ʧ�ܷ���FALSE(0x00) */
unsigned char Socket_Connect ( SOCKET s ) {
    Write_W5500_SOCK_1Byte ( s, Sn_MR, MR_TCP ); /* ����socketΪTCPģʽ */
    Write_W5500_SOCK_1Byte ( s, Sn_CR, OPEN ); /* ��Socket */
    Delay ( 5 ); /* ��ʱ5ms */

    if ( Read_W5500_SOCK_1Byte ( s, Sn_SR ) != SOCK_INIT ) { /* ���socket��ʧ�� */
        Write_W5500_SOCK_1Byte ( s, Sn_CR, CLOSE ); /* ����򿪲��ɹ����ر�Socket */
        return FALSE; /* ����FALSE(0x00) */
    }

    Write_W5500_SOCK_1Byte ( s, Sn_CR, CONNECT ); /* ����SocketΪConnectģʽ */
    return TRUE; /* ����TRUE�����óɹ� */
}

/* ����ָ��Socket(0~7)��Ϊ�������ȴ�Զ�����������ӣ�s�Ǵ��趨�Ķ˿ڡ��ɹ�����TRUE(0xFF)��ʧ�ܷ���FALSE(0x00) */
unsigned char Socket_Listen ( SOCKET s ) {
    Write_W5500_SOCK_1Byte ( s, Sn_MR, MR_TCP ); /* ����socketΪTCPģʽ */
    Write_W5500_SOCK_1Byte ( s, Sn_CR, OPEN ); /* ��Socket */
    Delay ( 5 ); /* ��ʱ5ms */

    if ( Read_W5500_SOCK_1Byte ( s, Sn_SR ) != SOCK_INIT ) { /* ���socket��ʧ�� */
        Write_W5500_SOCK_1Byte ( s, Sn_CR, CLOSE ); /* �򿪲��ɹ����ر�Socket */
        return FALSE; /* ����FALSE(0x00) */
    }

    Write_W5500_SOCK_1Byte ( s, Sn_CR, LISTEN ); /* ����SocketΪ����ģʽ */
    Delay ( 5 ); /* ��ʱ5ms */

    if ( Read_W5500_SOCK_1Byte ( s, Sn_SR ) != SOCK_LISTEN ) { /* ���socket����ʧ�� */
        Write_W5500_SOCK_1Byte ( s, Sn_CR, CLOSE ); /* ���ò��ɹ����ر�Socket */
        return FALSE; /* ����FALSE(0x00) */
    }

    return TRUE;
}

/* ����ָ��Socket(0~7)ΪUDPģʽ������s�Ǵ��趨�Ķ˿ڡ��ɹ�����TRUE(0xFF)��ʧ�ܷ���FALSE(0x00) */
unsigned char Socket_UDP ( SOCKET s ) {
    Write_W5500_SOCK_1Byte ( s, Sn_MR, MR_UDP ); //����SocketΪUDPģʽ*/
    Write_W5500_SOCK_1Byte ( s, Sn_CR, OPEN ); /* ��Socket*/
    Delay ( 5 ); /* ��ʱ5ms */

    if ( Read_W5500_SOCK_1Byte ( s, Sn_SR ) != SOCK_UDP ) { /* ���Socket��ʧ�� */
        Write_W5500_SOCK_1Byte ( s, Sn_CR, CLOSE ); /* �򿪲��ɹ����ر�Socket */
        return FALSE; /* ����FALSE(0x00) */
    } else {
        return TRUE;
    }
}

void W5500_Interrupt_Process ( void ) { /* W5500�жϴ�������� */
    unsigned char i, j;
IntDispose:
    i = Read_W5500_1Byte ( SIR ); /* ��ȡ�˿��жϱ�־�Ĵ��� */

    if ( ( i & S0_INT ) == S0_INT ) { /* Socket0�¼����� */
        j = Read_W5500_SOCK_1Byte ( 0, Sn_IR ); /* ��ȡSocket0�жϱ�־�Ĵ��� */
        Write_W5500_SOCK_1Byte ( 0, Sn_IR, j );

        if ( j & IR_CON ) { /* ��TCPģʽ�£�Socket0�ɹ����� */
            S0_State |= S_CONN; /* ��������״̬0x02���˿�������ӣ����������������� */
        }

        if ( j & IR_DISCON ) { /* ��TCPģʽ��Socket�Ͽ����Ӵ��� */
            Write_W5500_SOCK_1Byte ( 0, Sn_CR, CLOSE ); /* �رն˿ڣ��ȴ����´����� */
            Socket_Init ( 0 ); /* ָ��Socket(0~7)��ʼ������ʼ���˿�0 */
            S0_State = 0; /* ��������״̬0x00���˿�����ʧ�� */
        }

        if ( j & IR_SEND_OK ) { /* Socket0���ݷ�����ɣ������ٴ�����S_tx_process������������ */
            S0_Data |= S_TRANSMITOK; /* �˿ڷ���һ�����ݰ���� */
        }

        if ( j & IR_RECV ) { /* Socket���յ����ݣ���������S_rx_process���� */
            S0_Data |= S_RECEIVE; /* �˿ڽ��յ�һ�����ݰ� */
        }

        if ( j & IR_TIMEOUT ) { /* Socket���ӻ����ݴ��䳬ʱ���� */
            Write_W5500_SOCK_1Byte ( 0, Sn_CR, CLOSE ); /* �رն˿ڣ��ȴ����´����� */
            S0_State = 0; /* ��������״̬0x00���˿�����ʧ�� */
        }
    }

    if ( Read_W5500_1Byte ( SIR ) != 0 ) {
        goto IntDispose;
    }
}