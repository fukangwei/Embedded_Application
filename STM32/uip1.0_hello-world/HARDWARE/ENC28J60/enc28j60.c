#include "spi.h"
#include "delay.h"
#include "timerx.h"
#include <stdio.h>
#include "enc28j60.h"

static u8 ENC28J60BANK;
static u32 NextPacketPtr;

static void ENC28J60_SPI2_Init ( void ) {
    SPI_InitTypeDef SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB1PeriphClockCmd ( RCC_APB1Periph_SPI2, ENABLE ); /* SPI2时钟使能 */
    RCC_APB2PeriphClockCmd (  RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE );
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* 推挽输出 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; /* IO口速度为50MHz */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_0;
    GPIO_Init ( GPIOB, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOB, GPIO_Pin_12 | GPIO_Pin_0 );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_Init ( GPIOB, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOB, GPIO_Pin_1 );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
    GPIO_ResetBits ( GPIOC, GPIO_Pin_4 );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  /* 复用推挽输出 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOB, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOB, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15 );
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  /* 设置SPI单向或者双向的数据模式：SPI设置为双线双向全双工 */
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master; /* 设置SPI工作模式：设置为主SPI */
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; /* 设置SPI的数据大小：SPI发送接收8位帧结构 */
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; /* 串行同步时钟的空闲状态为低电平 */
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; /* 串行同步时钟的第一个跳变沿(上升或下降)数据被采样 */
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; /* NSS信号由硬件(NSS管脚)还是软件(使用SSI位)管理：内部NSS信号有SSI位控制 */
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128; /* 定义波特率预分频的值：波特率预分频值为256 */
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; /* 指定数据传输从MSB位还是LSB位开始：数据传输从MSB位开始 */
    SPI_InitStructure.SPI_CRCPolynomial = 7; /* CRC值计算的多项式 */
    SPI_Init ( SPI2, &SPI_InitStructure ); /* 根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器 */
    SPI_Cmd ( SPI2, ENABLE ); /* 使能SPI外设 */
    SPI2_ReadWriteByte ( 0xff ); /* 启动传输 */
}

void ENC28J60_Reset ( void ) {
    ENC28J60_SPI2_Init(); /* SPI2初始化 */
    SPI2_SetSpeed ( SPI_BaudRatePrescaler_4 ); /* SPI2的SCK频率为“36M / 4 = 9Mhz” */
    TIM6_Int_Init ( 1000, 719 ); /* 100Khz计数频率，计数到1000为10ms */
    ENC28J60_RST = 0; /* 复位ENC28J60 */
    delay_ms ( 10 );
    ENC28J60_RST = 1; /* 复位结束 */
    delay_ms ( 10 );
}

u8 ENC28J60_Read_Op ( u8 op, u8 addr ) { /* 读取ENC28J60寄存器(带操作码)，参数op操作码，addr是寄存器地址 */
    u8 dat = 0;
    ENC28J60_CS = 0;
    dat = op | ( addr & ADDR_MASK );
    SPI2_ReadWriteByte ( dat );
    dat = SPI2_ReadWriteByte ( 0xFF );

    if ( addr & 0x80 ) { /* 如果是读取MAC/MII寄存器，则第二次读到的数据才是正确的 */
        dat = SPI2_ReadWriteByte ( 0xFF );
    }

    ENC28J60_CS = 1;
    return dat;
}

void ENC28J60_Write_Op ( u8 op, u8 addr, u8 data ) { /* 读取ENC28J60寄存器(带操作码)，参数op是操作码，addr是寄存器地址，data是参数 */
    u8 dat = 0;
    ENC28J60_CS = 0;
    dat = op | ( addr & ADDR_MASK );
    SPI2_ReadWriteByte ( dat );
    SPI2_ReadWriteByte ( data );
    ENC28J60_CS = 1;
}

/* 读取ENC28J60接收缓存数据，参数len是要读取的数据长度，data是输出数据缓存区(末尾自动添加结束符) */
void ENC28J60_Read_Buf ( u32 len, u8 *data ) {
    ENC28J60_CS = 0;
    SPI2_ReadWriteByte ( ENC28J60_READ_BUF_MEM );

    while ( len ) {
        len--;
        *data = ( u8 ) SPI2_ReadWriteByte ( 0 );
        data++;
    }

    *data = '\0';
    ENC28J60_CS = 1;
}

void ENC28J60_Write_Buf ( u32 len, u8 *data ) { /* 向ENC28J60写发送缓存数据，参数len是要写入的数据长度，data是数据缓存区 */
    ENC28J60_CS = 0;
    SPI2_ReadWriteByte ( ENC28J60_WRITE_BUF_MEM );

    while ( len ) {
        len--;
        SPI2_ReadWriteByte ( *data );
        data++;
    }

    ENC28J60_CS = 1;
}

void ENC28J60_Set_Bank ( u8 bank ) { /* 设置ENC28J60寄存器Bank，参数ban是要设置的bank */
    if ( ( bank & BANK_MASK ) != ENC28J60BANK ) { /* 和当前bank不一致时设置 */
        ENC28J60_Write_Op ( ENC28J60_BIT_FIELD_CLR, ECON1, ( ECON1_BSEL1 | ECON1_BSEL0 ) );
        ENC28J60_Write_Op ( ENC28J60_BIT_FIELD_SET, ECON1, ( bank & BANK_MASK ) >> 5 );
        ENC28J60BANK = ( bank & BANK_MASK );
    }
}

u8 ENC28J60_Read ( u8 addr ) { /* 读取ENC28J60指定寄存器，参数addr是寄存器地址 */
    ENC28J60_Set_Bank ( addr ); /* 设置BANK */
    return ENC28J60_Read_Op ( ENC28J60_READ_CTRL_REG, addr );
}

void ENC28J60_Write ( u8 addr, u8 data ) { /* 向ENC28J60指定寄存器写数据。参数addr是寄存器地址，data是要写入的数据 */
    ENC28J60_Set_Bank ( addr );
    ENC28J60_Write_Op ( ENC28J60_WRITE_CTRL_REG, addr, data );
}

void ENC28J60_PHY_Write ( u8 addr, u32 data ) { /* 向ENC28J60的PHY寄存器写入数据。参数addr是寄存器地址，data是要写入的数据 */
    u16 retry = 0;
    ENC28J60_Write ( MIREGADR, addr ); /* 设置PHY寄存器地址 */
    ENC28J60_Write ( MIWRL, data ); /* 写入数据 */
    ENC28J60_Write ( MIWRH, data >> 8 );

    while ( ( ENC28J60_Read ( MISTAT ) &MISTAT_BUSY ) && retry < 0XFFF ) {
        retry++; /* 等待写入PHY结束 */
    }
}

u8 ENC28J60_Init ( u8 *macaddr ) { /* 初始化ENC28J60，参数macaddr是MAC地址 */
    u16 retry = 0;
    ENC28J60_Reset();
    ENC28J60_Write_Op ( ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET ); /* 软件复位 */

    while ( ! ( ENC28J60_Read ( ESTAT ) &ESTAT_CLKRDY ) && retry < 500 ) { /* 等待时钟稳定 */
        retry++;
        delay_ms ( 1 );
    };

    if ( retry >= 500 ) {
        return 1; /* ENC28J60初始化失败 */
    }

    NextPacketPtr = RXSTART_INIT;
    ENC28J60_Write ( ERXSTL, RXSTART_INIT & 0xFF );
    ENC28J60_Write ( ERXSTH, RXSTART_INIT >> 8 );
    ENC28J60_Write ( ERXRDPTL, RXSTART_INIT & 0xFF );
    ENC28J60_Write ( ERXRDPTH, RXSTART_INIT >> 8 );
    
    ENC28J60_Write ( ERXNDL, RXSTOP_INIT & 0xFF );
    ENC28J60_Write ( ERXNDH, RXSTOP_INIT >> 8 );
    
    ENC28J60_Write ( ETXSTL, TXSTART_INIT & 0xFF );
    ENC28J60_Write ( ETXSTH, TXSTART_INIT >> 8 );
    
    ENC28J60_Write ( ETXNDL, TXSTOP_INIT & 0xFF );
    ENC28J60_Write ( ETXNDH, TXSTOP_INIT >> 8 );

    ENC28J60_Write ( ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN );
    ENC28J60_Write ( EPMM0, 0x3f );
    ENC28J60_Write ( EPMM1, 0x30 );
    ENC28J60_Write ( EPMCSL, 0xf9 );
    ENC28J60_Write ( EPMCSH, 0xf7 );

    ENC28J60_Write ( MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS );

    ENC28J60_Write ( MACON2, 0x00 );
   
    ENC28J60_Write_Op ( ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX );

    ENC28J60_Write ( MAIPGL, 0x12 );
    ENC28J60_Write ( MAIPGH, 0x0C );

    ENC28J60_Write ( MABBIPG, 0x15 );

    ENC28J60_Write ( MAMXFLL, MAX_FRAMELEN & 0xFF );
    ENC28J60_Write ( MAMXFLH, MAX_FRAMELEN >> 8 );

    ENC28J60_Write ( MAADR5, macaddr[0] );
    ENC28J60_Write ( MAADR4, macaddr[1] );
    ENC28J60_Write ( MAADR3, macaddr[2] );
    ENC28J60_Write ( MAADR2, macaddr[3] );
    ENC28J60_Write ( MAADR1, macaddr[4] );
    ENC28J60_Write ( MAADR0, macaddr[5] );
   
    ENC28J60_PHY_Write ( PHCON1, PHCON1_PDPXMD );

    ENC28J60_PHY_Write ( PHCON2, PHCON2_HDLDIS );

    ENC28J60_Set_Bank ( ECON1 );

    ENC28J60_Write_Op ( ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE );

    ENC28J60_Write_Op ( ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN );

    if ( ENC28J60_Read ( MAADR5 ) == macaddr[0] ) {
        return 0;   
    } else {
        return 1;
    }
}

u8 ENC28J60_Get_EREVID ( void ) { /* 读取EREVID */
    return ENC28J60_Read ( EREVID );
}

#include "uip.h"
void ENC28J60_Packet_Send ( u32 len, u8 *packet ) { /* 通过ENC28J60发送数据包到网络，参数len是数据包大小，packet是数据包 */
    /* 设置发送缓冲区地址写指针入口 */
    ENC28J60_Write ( EWRPTL, TXSTART_INIT & 0xFF );
    ENC28J60_Write ( EWRPTH, TXSTART_INIT >> 8 );
    /* 设置TXND指针，以对应给定的数据包大小 */
    ENC28J60_Write ( ETXNDL, ( TXSTART_INIT + len ) & 0xFF );
    ENC28J60_Write ( ETXNDH, ( TXSTART_INIT + len ) >> 8 );
    ENC28J60_Write_Op ( ENC28J60_WRITE_BUF_MEM, 0, 0x00 ); /* 写每包控制字节(0x00表示使用macon3的设置) */
    ENC28J60_Write_Buf ( len, packet ); /* 复制数据包到发送缓冲区 */
    ENC28J60_Write_Op ( ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS ); /* 发送数据到网络 */

    if ( ( ENC28J60_Read ( EIR ) &EIR_TXERIF ) ) {
        ENC28J60_Write_Op ( ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS );
    }
}

/* 从网络获取一个数据包内容。参数maxlen是数据包最大允许接收长度，packet是数据包缓存区。函数返回收到的数据包长度(字节) */
u32 ENC28J60_Packet_Receive ( u32 maxlen, u8 *packet ) {
    u32 rxstat;
    u32 len;

    if ( ENC28J60_Read ( EPKTCNT ) == 0 ) {
        return 0;
    }

    /* 设置接收缓冲器读指针 */
    ENC28J60_Write ( ERDPTL, ( NextPacketPtr ) );
    ENC28J60_Write ( ERDPTH, ( NextPacketPtr ) >> 8 );
    /* 读下一个包的指针 */
    NextPacketPtr = ENC28J60_Read_Op ( ENC28J60_READ_BUF_MEM, 0 );
    NextPacketPtr |= ENC28J60_Read_Op ( ENC28J60_READ_BUF_MEM, 0 ) << 8;
    /* 读包的长度 */
    len = ENC28J60_Read_Op ( ENC28J60_READ_BUF_MEM, 0 );
    len |= ENC28J60_Read_Op ( ENC28J60_READ_BUF_MEM, 0 ) << 8;
    len -= 4; /* 去掉CRC计数 */
    /* 读取接收状态 */
    rxstat = ENC28J60_Read_Op ( ENC28J60_READ_BUF_MEM, 0 );
    rxstat |= ENC28J60_Read_Op ( ENC28J60_READ_BUF_MEM, 0 ) << 8;

    if ( len > maxlen - 1 ) { /* 限制接收长度 */
        len = maxlen - 1;
    }

    if ( ( rxstat & 0x80 ) == 0 ) {
        len = 0;   
    } else {
        ENC28J60_Read_Buf ( len, packet ); /* 从接收缓冲器中复制数据包 */
    }

    /* RX读指针移动到下一个接收到的数据包的开始位置，并释放我们刚才读出过的内存 */
    ENC28J60_Write ( ERXRDPTL, ( NextPacketPtr ) );
    ENC28J60_Write ( ERXRDPTH, ( NextPacketPtr ) >> 8 );
    ENC28J60_Write_Op ( ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC );
    return ( len );
}