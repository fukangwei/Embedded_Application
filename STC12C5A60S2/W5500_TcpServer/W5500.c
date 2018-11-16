#include "W5500.h"

unsigned char Gateway_IP[4]; /* 网关IP地址 */
unsigned char Sub_Mask[4];   /* 子网掩码 */
unsigned char Phy_Addr[6];   /* 物理地址(MAC) */
unsigned char IP_Addr[4];    /* 本机IP地址 */
unsigned char S0_Port[2];    /* 端口0的端口号 */
unsigned char S0_DIP[4];     /* 端口0目的IP地址 */
unsigned char S0_DPort[2];   /* 端口0目的端口号 */
unsigned char UDP_DIPR[4];   /* UDP(广播)模式的目的主机IP地址 */
unsigned char UDP_DPORT[2];  /* UDP(广播)模式的目的主机端口号 */

unsigned char S0_Mode = 3;  /* 端口0的运行模式 */

#define TCP_SERVER 0x00 /* TCP服务器模式 */
#define TCP_CLIENT 0x01 /* TCP客户端模式 */
#define UDP_MODE   0x02 /* UDP(广播)模式 */

unsigned char S0_State = 0; /* 端口0状态记录 */
#define S_INIT 0x01 /* 端口完成初始化 */
#define S_CONN 0x02 /* 端口完成连接，可以正常传输数据 */

unsigned char S0_Data; /* 端口0接收和发送数据的状态 */
#define S_RECEIVE    0x01 /* 端口接收到一个数据包 */
#define S_TRANSMITOK 0x02 /* 端口发送一个数据包完成 */

unsigned char Rx_Buffer[30]; /* 端口接收数据缓冲区 */
unsigned char Tx_Buffer[30]; /* 端口发送数据缓冲区 */
unsigned char W5500_Interrupt; /* W5500中断标志(0是无中断，1是有中断) */

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
    SPI_Send_Byte ( ( unsigned char ) ( dt / 256 ) ); /* 写数据高位 */
    SPI_Send_Byte ( dt ); /* 写数据低位 */
}

void Write_W5500_1Byte ( unsigned short reg, unsigned char dat ) {
    W5500_SCS = 0; /* 置W5500的SCS为低电平 */
    SPI_Send_Short ( reg ); /* 通过SPI写16位寄存器地址 */
    SPI_Send_Byte ( FDM1 | RWB_WRITE | COMMON_R ); /* 通过SPI写控制字节，1个字节数据长度，写数据，选择通用寄存器 */
    SPI_Send_Byte ( dat ); /* 写1个字节数据 */
    W5500_SCS = 1; /* 置W5500的SCS为高电平 */
}

void Write_W5500_2Byte ( unsigned short reg, unsigned short dat ) {
    W5500_SCS = 0; /* 置W5500的SCS为低电平 */
    SPI_Send_Short ( reg ); /* 通过SPI写16位寄存器地址 */
    SPI_Send_Byte ( FDM2 | RWB_WRITE | COMMON_R ); /* 通过SPI写控制字节，2个字节数据长度，写数据，选择通用寄存器 */
    SPI_Send_Short ( dat ); /* 写16位数据 */
    W5500_SCS = 1; /* 置W5500的SCS为高电平 */
}

void Write_W5500_nByte ( unsigned short reg, unsigned char *dat_ptr, unsigned short size ) {
    unsigned short i;
    W5500_SCS = 0; /* 置W5500的SCS为低电平 */
    SPI_Send_Short ( reg ); /* 通过SPI写16位寄存器地址 */
    SPI_Send_Byte ( VDM | RWB_WRITE | COMMON_R ); /* 通过SPI写控制字节，N个字节数据长度，写数据，选择通用寄存器 */

    for ( i = 0; i < size; i++ ) { /* 循环将缓冲区的size个字节数据写入W5500 */
        SPI_Send_Byte ( *dat_ptr++ ); /* 写一个字节数据 */
    }

    W5500_SCS = 1; /* 置W5500的SCS为高电平 */
}

void Write_W5500_SOCK_1Byte ( SOCKET s, unsigned short reg, unsigned char dat ) {
    W5500_SCS = 0; /* 置W5500的SCS为低电平 */
    SPI_Send_Short ( reg ); /* 通过SPI写16位寄存器地址 */
    SPI_Send_Byte ( FDM1 | RWB_WRITE | ( s * 0x20 + 0x08 ) ); /* 通过SPI写控制字节，1个字节数据长度，写数据，选择端口s的寄存器 */
    SPI_Send_Byte ( dat ); /* 写1个字节数据 */
    W5500_SCS = 1; /* 置W5500的SCS为高电平 */
}

/* 通过SPI向指定端口寄存器写2个字节数据。参数s是端口号，reg是16位寄存器地址，dat是16位待写入的数据(2个字节) */
void Write_W5500_SOCK_2Byte ( SOCKET s, unsigned short reg, unsigned short dat ) {
    W5500_SCS = 0; /* 置W5500的SCS为低电平 */
    SPI_Send_Short ( reg ); /* 通过SPI写16位寄存器地址 */
    SPI_Send_Byte ( FDM2 | RWB_WRITE | ( s * 0x20 + 0x08 ) ); /* 通过SPI写控制字节，2个字节数据长度，写数据，选择端口s的寄存器 */
    SPI_Send_Short ( dat ); /* 写16位数据 */
    W5500_SCS = 1; /* 置W5500的SCS为高电平 */
}

/* 通过SPI向指定端口寄存器写4个字节数据。参数s是端口号，reg是16位寄存器地址，dat_ptr是待写入的4个字节缓冲区指针 */
void Write_W5500_SOCK_4Byte ( SOCKET s, unsigned short reg, unsigned char *dat_ptr ) {
    W5500_SCS = 0; /* 置W5500的SCS为低电平 */
    SPI_Send_Short ( reg ); /* 通过SPI写16位寄存器地址 */
    SPI_Send_Byte ( FDM4 | RWB_WRITE | ( s * 0x20 + 0x08 ) ); /* 通过SPI写控制字节，4个字节数据长度，写数据，选择端口s的寄存器 */
    SPI_Send_Byte ( *dat_ptr++ ); /* 写第1个字节数据 */
    SPI_Send_Byte ( *dat_ptr++ ); /* 写第2个字节数据 */
    SPI_Send_Byte ( *dat_ptr++ ); /* 写第3个字节数据 */
    SPI_Send_Byte ( *dat_ptr++ ); /* 写第4个字节数据 */
    W5500_SCS = 1; /* 置W5500的SCS为高电平 */
}

unsigned char Read_W5500_1Byte ( unsigned short reg ) { /* 读W5500指定地址寄存器的1个字节数据。参数reg是16位寄存器地址 */
    unsigned char i;
    W5500_SCS = 0; /* 置W5500的SCS为低电平 */
    SPI_Send_Short ( reg ); /* 通过SPI写16位寄存器地址 */
    SPI_Send_Byte ( FDM1 | RWB_READ | COMMON_R ); /* 通过SPI写控制字节，1个字节数据长度，读数据，选择通用寄存器 */
    i = SPI_Read_Byte();
    W5500_SCS = 1; /* 置W5500的SCS为高电平 */
    return i; /* 返回读取到的寄存器数据 */
}

/* 读W5500指定端口寄存器的1个字节数据。参数s是端口号，reg是16位寄存器地址 */
unsigned char Read_W5500_SOCK_1Byte ( SOCKET s, unsigned short reg ) {
    unsigned char i;
    W5500_SCS = 0; /* 置W5500的SCS为低电平 */
    SPI_Send_Short ( reg ); /* 通过SPI写16位寄存器地址 */
    SPI_Send_Byte ( FDM1 | RWB_READ | ( s * 0x20 + 0x08 ) ); /* 通过SPI写控制字节，1个字节数据长度，读数据，选择端口s的寄存器 */
    i = SPI_Read_Byte();
    W5500_SCS = 1; /* 置W5500的SCS为高电平 */
    return i; /* 返回读取到的寄存器数据 */
}

/* 读W5500指定端口寄存器的2个字节数据。参数s是端口号，reg是16位寄存器地址 */
unsigned short Read_W5500_SOCK_2Byte ( SOCKET s, unsigned short reg ) {
    unsigned short i;
    W5500_SCS = 0; /* 置W5500的SCS为低电平 */
    SPI_Send_Short ( reg ); /* 通过SPI写16位寄存器地址 */
    SPI_Send_Byte ( FDM2 | RWB_READ | ( s * 0x20 + 0x08 ) ); /* 通过SPI写控制字节，2个字节数据长度，读数据，选择端口s的寄存器 */
    i = SPI_Read_Byte();
    i *= 256;
    i += SPI_Read_Byte(); /* 读取低位数据 */
    W5500_SCS = 1; /* 置W5500的SCS为高电平 */
    return i; /* 返回读取到的寄存器数据 */
}

/* 从W5500接收数据缓冲区中读取数据。参数s是端口号，dat_ptr是数据保存缓冲区指针 */
unsigned short Read_SOCK_Data_Buffer ( SOCKET s, unsigned char *dat_ptr ) {
    unsigned short rx_size;
    unsigned short offset, offset1;
    unsigned short i;
    unsigned char j;
    rx_size = Read_W5500_SOCK_2Byte ( s, Sn_RX_RSR );

    if ( rx_size == 0 ) {
        return 0; /* 没接收到数据则返回 */
    }

    if ( rx_size > 1460 ) {
        rx_size = 1460;
    }

    offset = Read_W5500_SOCK_2Byte ( s, Sn_RX_RD );
    offset1 = offset;
    offset &= ( S_RX_SIZE - 1 ); /* 计算实际的物理地址 */
    W5500_SCS = 0; /* 置W5500的SCS为低电平 */
    SPI_Send_Short ( offset ); /* 写16位地址 */
    SPI_Send_Byte ( VDM | RWB_READ | ( s * 0x20 + 0x18 ) ); /* 写控制字节，N个字节数据长度，读数据，选择端口s的寄存器 */

    if ( ( offset + rx_size ) < S_RX_SIZE ) { /* 如果最大地址未超过W5500接收缓冲区寄存器的最大地址 */
        for ( i = 0; i < rx_size; i++ ) { /* 循环读取rx_size个字节数据 */
            j = SPI_Read_Byte(); /* 读取1个字节数据 */
            *dat_ptr = j; /* 将读取到的数据保存到数据保存缓冲区 */
            dat_ptr++; /* 数据保存缓冲区指针地址自增1 */
        }
    } else { /* 如果最大地址超过W5500接收缓冲区寄存器的最大地址 */
        offset = S_RX_SIZE - offset;

        for ( i = 0; i < offset; i++ ) { /* 循环读取出前offset个字节数据 */
            j = SPI_Read_Byte(); /* 读取1个字节数据 */
            *dat_ptr = j; /* 将读取到的数据保存到数据保存缓冲区 */
            dat_ptr++; /* 数据保存缓冲区指针地址自增1 */
        }

        W5500_SCS = 1; /* 置W5500的SCS为高电平 */
        W5500_SCS = 0; /* 置W5500的SCS为低电平 */
        SPI_Send_Short ( 0x00 ); /* 写16位地址 */
        SPI_Send_Byte ( VDM | RWB_READ | ( s * 0x20 + 0x18 ) ); /* 写控制字节，N个字节数据长度，读数据，选择端口s的寄存器 */

        for ( ; i < rx_size; i++ ) { /* 循环读取后“rx_size - offset”个字节数据 */
            j = SPI_Read_Byte(); /* 读取1个字节数据 */
            *dat_ptr = j; /* 将读取到的数据保存到数据保存缓冲区 */
            dat_ptr++; /* 数据保存缓冲区指针地址自增1 */
        }
    }

    W5500_SCS = 1; /* 置W5500的SCS为高电平 */
    offset1 += rx_size; /* 更新实际物理地址，即下次读取接收到的数据的起始地址 */
    Write_W5500_SOCK_2Byte ( s, Sn_RX_RD, offset1 );
    Write_W5500_SOCK_1Byte ( s, Sn_CR, RECV ); /* 发送启动接收命令 */
    return rx_size; /* 返回接收到数据的长度 */
}

/* 将数据写入W5500的数据发送缓冲区。参数s是端口号，dat_ptr是数据保存缓冲区指针，size是待写入数据的长度 */
void Write_SOCK_Data_Buffer ( SOCKET s, unsigned char *dat_ptr, unsigned short size ) {
    unsigned short offset, offset1;
    unsigned short i;

    /* 如果是UDP模式，可以在此设置目的主机的IP和端口号 */
    if ( ( Read_W5500_SOCK_1Byte ( s, Sn_MR ) & 0x0f ) != SOCK_UDP ) { /* 如果Socket打开失败 */
        Write_W5500_SOCK_4Byte ( s, Sn_DIPR, UDP_DIPR ); /* 设置目的主机IP */
        Write_W5500_SOCK_2Byte ( s, Sn_DPORTR, UDP_DPORT[0] * 256 + UDP_DPORT[1] ); /* 设置目的主机端口号 */
    }

    offset = Read_W5500_SOCK_2Byte ( s, Sn_TX_WR );
    offset1 = offset;
    offset &= ( S_TX_SIZE - 1 ); /* 计算实际的物理地址 */
    W5500_SCS = 0; /* 置W5500的SCS为低电平 */
    SPI_Send_Short ( offset ); /* 写16位地址 */
    SPI_Send_Byte ( VDM | RWB_WRITE | ( s * 0x20 + 0x10 ) ); /* 写控制字节，N个字节数据长度，写数据，选择端口s的寄存器 */

    if ( ( offset + size ) < S_TX_SIZE ) { /* 如果最大地址未超过W5500发送缓冲区寄存器的最大地址 */
        for ( i = 0; i < size; i++ ) { /* 循环写入size个字节数据 */
            SPI_Send_Byte ( *dat_ptr++ ); /* 写入一个字节的数据 */
        }
    } else { /* 如果最大地址超过W5500发送缓冲区寄存器的最大地址 */
        offset = S_TX_SIZE - offset;

        for ( i = 0; i < offset; i++ ) { /* 循环写入前offset个字节数据 */
            SPI_Send_Byte ( *dat_ptr++ ); /* 写入一个字节的数据 */
        }

        W5500_SCS = 1; /* 置W5500的SCS为高电平 */
        W5500_SCS = 0; /* 置W5500的SCS为低电平 */
        SPI_Send_Short ( 0x00 ); /* 写16位地址 */
        SPI_Send_Byte ( VDM | RWB_WRITE | ( s * 0x20 + 0x10 ) ); /* 写控制字节，N个字节数据长度，写数据，选择端口s的寄存器 */

        for ( ; i < size; i++ ) { /* 循环写入“size - offset”个字节数据 */
            SPI_Send_Byte ( *dat_ptr++ ); /* 写入一个字节的数据 */
        }
    }

    W5500_SCS = 1; /* 置W5500的SCS为高电平 */
    offset1 += size; /* 更新实际物理地址即下次写待发送数据到发送数据缓冲区的起始地址 */
    Write_W5500_SOCK_2Byte ( s, Sn_TX_WR, offset1 );
    Write_W5500_SOCK_1Byte ( s, Sn_CR, SEND ); /* 发送启动发送命令 */
}

void W5500_Hardware_Reset ( void ) {
    W5500_RST = 0; /* 复位引脚拉低 */
    Delay ( 200 );
    W5500_RST = 1; /* 复位引脚拉高 */
    Delay ( 200 );

    while ( ( Read_W5500_1Byte ( PHYCFGR ) &LINK ) == 0 ); /* 等待以太网连接完成 */
}

void W5500_Init ( void ) {
    unsigned char i = 0;
    Write_W5500_1Byte ( MR, RST ); /* 软件复位W5500，置1有效，复位后自动清0 */
    Delay ( 10 ); /* 延时10ms */
    Write_W5500_nByte ( GAR, Gateway_IP, 4 );
    Write_W5500_nByte ( SUBR, Sub_Mask, 4 );
    Write_W5500_nByte ( SHAR, Phy_Addr, 6 );
    Write_W5500_nByte ( SIPR, IP_Addr, 4 );

    for ( i = 0; i < 8; i++ ) { /* 设置发送缓冲区和接收缓冲区的大小 */
        Write_W5500_SOCK_1Byte ( i, Sn_RXBUF_SIZE, 0x02 ); /* Socket Rx memory size = 2k */
        Write_W5500_SOCK_1Byte ( i, Sn_TXBUF_SIZE, 0x02 ); /* Socket Tx mempry size = 2k */
    }

    Write_W5500_2Byte ( RTR, 0x07d0 );
    Write_W5500_1Byte ( RCR, 8 );
}

unsigned char Detect_Gateway ( void ) { /* 检查网关服务器。成功返回TRUE(0xFF)，失败返回FALSE(0x00) */
    unsigned char ip_adde[4];
    ip_adde[0] = IP_Addr[0] + 1;
    ip_adde[1] = IP_Addr[1] + 1;
    ip_adde[2] = IP_Addr[2] + 1;
    ip_adde[3] = IP_Addr[3] + 1;
    /* 检查网关及获取网关的物理地址 */
    Write_W5500_SOCK_4Byte ( 0, Sn_DIPR, ip_adde ); /* 向目的地址寄存器写入与本机IP不同的IP值 */
    Write_W5500_SOCK_1Byte ( 0, Sn_MR, MR_TCP ); /* 设置socket为TCP模式 */
    Write_W5500_SOCK_1Byte ( 0, Sn_CR, OPEN ); /* 打开Socket */
    Delay ( 5 ); /* 延时5ms */

    if ( Read_W5500_SOCK_1Byte ( 0, Sn_SR ) != SOCK_INIT ) { /* 如果socket打开失败 */
        Write_W5500_SOCK_1Byte ( 0, Sn_CR, CLOSE ); /* 打开不成功，关闭Socket */
        return FALSE; /* 返回FALSE(0x00) */
    }

    Write_W5500_SOCK_1Byte ( 0, Sn_CR, CONNECT ); /* 设置Socket为Connect模式 */

    do {
        unsigned char j = 0;
        j = Read_W5500_SOCK_1Byte ( 0, Sn_IR ); /* 读取Socket0中断标志寄存器 */

        if ( j != 0 ) {
            Write_W5500_SOCK_1Byte ( 0, Sn_IR, j );
        }

        Delay ( 5 ); /* 延时5ms */

        if ( ( j & IR_TIMEOUT ) == IR_TIMEOUT ) {
            return FALSE;
        } else if ( Read_W5500_SOCK_1Byte ( 0, Sn_DHAR ) != 0xff ) {
            Write_W5500_SOCK_1Byte ( 0, Sn_CR, CLOSE ); /* 关闭Socket */
            return TRUE;
        }
    } while ( 1 );
}

void Socket_Init ( SOCKET s ) { /* 指定Socket(0~7)初始化。参数s是待初始化的端口 */
    Write_W5500_SOCK_2Byte ( 0, Sn_MSSR, 30 ); /* 最大分片字节数 */

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

/* 设置指定Socket(0~7)为客户端与远程服务器连接，参数s是待设定的端口。成功返回TRUE(0xFF)，失败返回FALSE(0x00) */
unsigned char Socket_Connect ( SOCKET s ) {
    Write_W5500_SOCK_1Byte ( s, Sn_MR, MR_TCP ); /* 设置socket为TCP模式 */
    Write_W5500_SOCK_1Byte ( s, Sn_CR, OPEN ); /* 打开Socket */
    Delay ( 5 ); /* 延时5ms */

    if ( Read_W5500_SOCK_1Byte ( s, Sn_SR ) != SOCK_INIT ) { /* 如果socket打开失败 */
        Write_W5500_SOCK_1Byte ( s, Sn_CR, CLOSE ); /* 如果打开不成功，关闭Socket */
        return FALSE; /* 返回FALSE(0x00) */
    }

    Write_W5500_SOCK_1Byte ( s, Sn_CR, CONNECT ); /* 设置Socket为Connect模式 */
    return TRUE; /* 返回TRUE，设置成功 */
}

/* 设置指定Socket(0~7)作为服务器等待远程主机的连接，s是待设定的端口。成功返回TRUE(0xFF)，失败返回FALSE(0x00) */
unsigned char Socket_Listen ( SOCKET s ) {
    Write_W5500_SOCK_1Byte ( s, Sn_MR, MR_TCP ); /* 设置socket为TCP模式 */
    Write_W5500_SOCK_1Byte ( s, Sn_CR, OPEN ); /* 打开Socket */
    Delay ( 5 ); /* 延时5ms */

    if ( Read_W5500_SOCK_1Byte ( s, Sn_SR ) != SOCK_INIT ) { /* 如果socket打开失败 */
        Write_W5500_SOCK_1Byte ( s, Sn_CR, CLOSE ); /* 打开不成功，关闭Socket */
        return FALSE; /* 返回FALSE(0x00) */
    }

    Write_W5500_SOCK_1Byte ( s, Sn_CR, LISTEN ); /* 设置Socket为侦听模式 */
    Delay ( 5 ); /* 延时5ms */

    if ( Read_W5500_SOCK_1Byte ( s, Sn_SR ) != SOCK_LISTEN ) { /* 如果socket设置失败 */
        Write_W5500_SOCK_1Byte ( s, Sn_CR, CLOSE ); /* 设置不成功，关闭Socket */
        return FALSE; /* 返回FALSE(0x00) */
    }

    return TRUE;
}

/* 设置指定Socket(0~7)为UDP模式，参数s是待设定的端口。成功返回TRUE(0xFF)，失败返回FALSE(0x00) */
unsigned char Socket_UDP ( SOCKET s ) {
    Write_W5500_SOCK_1Byte ( s, Sn_MR, MR_UDP ); //设置Socket为UDP模式*/
    Write_W5500_SOCK_1Byte ( s, Sn_CR, OPEN ); /* 打开Socket*/
    Delay ( 5 ); /* 延时5ms */

    if ( Read_W5500_SOCK_1Byte ( s, Sn_SR ) != SOCK_UDP ) { /* 如果Socket打开失败 */
        Write_W5500_SOCK_1Byte ( s, Sn_CR, CLOSE ); /* 打开不成功，关闭Socket */
        return FALSE; /* 返回FALSE(0x00) */
    } else {
        return TRUE;
    }
}

void W5500_Interrupt_Process ( void ) { /* W5500中断处理程序框架 */
    unsigned char i, j;
IntDispose:
    i = Read_W5500_1Byte ( SIR ); /* 读取端口中断标志寄存器 */

    if ( ( i & S0_INT ) == S0_INT ) { /* Socket0事件处理 */
        j = Read_W5500_SOCK_1Byte ( 0, Sn_IR ); /* 读取Socket0中断标志寄存器 */
        Write_W5500_SOCK_1Byte ( 0, Sn_IR, j );

        if ( j & IR_CON ) { /* 在TCP模式下，Socket0成功连接 */
            S0_State |= S_CONN; /* 网络连接状态0x02，端口完成连接，可以正常传输数据 */
        }

        if ( j & IR_DISCON ) { /* 在TCP模式下Socket断开连接处理 */
            Write_W5500_SOCK_1Byte ( 0, Sn_CR, CLOSE ); /* 关闭端口，等待重新打开连接 */
            Socket_Init ( 0 ); /* 指定Socket(0~7)初始化，初始化端口0 */
            S0_State = 0; /* 网络连接状态0x00，端口连接失败 */
        }

        if ( j & IR_SEND_OK ) { /* Socket0数据发送完成，可以再次启动S_tx_process函数发送数据 */
            S0_Data |= S_TRANSMITOK; /* 端口发送一个数据包完成 */
        }

        if ( j & IR_RECV ) { /* Socket接收到数据，可以启动S_rx_process函数 */
            S0_Data |= S_RECEIVE; /* 端口接收到一个数据包 */
        }

        if ( j & IR_TIMEOUT ) { /* Socket连接或数据传输超时处理 */
            Write_W5500_SOCK_1Byte ( 0, Sn_CR, CLOSE ); /* 关闭端口，等待重新打开连接 */
            S0_State = 0; /* 网络连接状态0x00，端口连接失败 */
        }
    }

    if ( Read_W5500_1Byte ( SIR ) != 0 ) {
        goto IntDispose;
    }
}