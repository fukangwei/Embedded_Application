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
    RCC_APB1PeriphClockCmd ( RCC_APB1Periph_SPI2, ENABLE ); /* SPI2ʱ��ʹ�� */
    RCC_APB2PeriphClockCmd (  RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE );
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* ������� */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; /* IO���ٶ�Ϊ50MHz */
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
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  /* ����������� */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOB, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOB, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15 );
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  /* ����SPI�������˫�������ģʽ��SPI����Ϊ˫��˫��ȫ˫�� */
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master; /* ����SPI����ģʽ������Ϊ��SPI */
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; /* ����SPI�����ݴ�С��SPI���ͽ���8λ֡�ṹ */
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; /* ����ͬ��ʱ�ӵĿ���״̬Ϊ�͵�ƽ */
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; /* ����ͬ��ʱ�ӵĵ�һ��������(�������½�)���ݱ����� */
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; /* NSS�ź���Ӳ��(NSS�ܽ�)�������(ʹ��SSIλ)�����ڲ�NSS�ź���SSIλ���� */
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128; /* ���岨����Ԥ��Ƶ��ֵ��������Ԥ��ƵֵΪ256 */
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; /* ָ�����ݴ����MSBλ����LSBλ��ʼ�����ݴ����MSBλ��ʼ */
    SPI_InitStructure.SPI_CRCPolynomial = 7; /* CRCֵ����Ķ���ʽ */
    SPI_Init ( SPI2, &SPI_InitStructure ); /* ����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ��� */
    SPI_Cmd ( SPI2, ENABLE ); /* ʹ��SPI���� */
    SPI2_ReadWriteByte ( 0xff ); /* �������� */
}

void ENC28J60_Reset ( void ) {
    ENC28J60_SPI2_Init(); /* SPI2��ʼ�� */
    SPI2_SetSpeed ( SPI_BaudRatePrescaler_4 ); /* SPI2��SCKƵ��Ϊ��36M / 4 = 9Mhz�� */
    TIM6_Int_Init ( 1000, 719 ); /* 100Khz����Ƶ�ʣ�������1000Ϊ10ms */
    ENC28J60_RST = 0; /* ��λENC28J60 */
    delay_ms ( 10 );
    ENC28J60_RST = 1; /* ��λ���� */
    delay_ms ( 10 );
}

u8 ENC28J60_Read_Op ( u8 op, u8 addr ) { /* ��ȡENC28J60�Ĵ���(��������)������op�����룬addr�ǼĴ�����ַ */
    u8 dat = 0;
    ENC28J60_CS = 0;
    dat = op | ( addr & ADDR_MASK );
    SPI2_ReadWriteByte ( dat );
    dat = SPI2_ReadWriteByte ( 0xFF );

    if ( addr & 0x80 ) { /* ����Ƕ�ȡMAC/MII�Ĵ�������ڶ��ζ��������ݲ�����ȷ�� */
        dat = SPI2_ReadWriteByte ( 0xFF );
    }

    ENC28J60_CS = 1;
    return dat;
}

void ENC28J60_Write_Op ( u8 op, u8 addr, u8 data ) { /* ��ȡENC28J60�Ĵ���(��������)������op�ǲ����룬addr�ǼĴ�����ַ��data�ǲ��� */
    u8 dat = 0;
    ENC28J60_CS = 0;
    dat = op | ( addr & ADDR_MASK );
    SPI2_ReadWriteByte ( dat );
    SPI2_ReadWriteByte ( data );
    ENC28J60_CS = 1;
}

/* ��ȡENC28J60���ջ������ݣ�����len��Ҫ��ȡ�����ݳ��ȣ�data��������ݻ�����(ĩβ�Զ���ӽ�����) */
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

void ENC28J60_Write_Buf ( u32 len, u8 *data ) { /* ��ENC28J60д���ͻ������ݣ�����len��Ҫд������ݳ��ȣ�data�����ݻ����� */
    ENC28J60_CS = 0;
    SPI2_ReadWriteByte ( ENC28J60_WRITE_BUF_MEM );

    while ( len ) {
        len--;
        SPI2_ReadWriteByte ( *data );
        data++;
    }

    ENC28J60_CS = 1;
}

void ENC28J60_Set_Bank ( u8 bank ) { /* ����ENC28J60�Ĵ���Bank������ban��Ҫ���õ�bank */
    if ( ( bank & BANK_MASK ) != ENC28J60BANK ) { /* �͵�ǰbank��һ��ʱ���� */
        ENC28J60_Write_Op ( ENC28J60_BIT_FIELD_CLR, ECON1, ( ECON1_BSEL1 | ECON1_BSEL0 ) );
        ENC28J60_Write_Op ( ENC28J60_BIT_FIELD_SET, ECON1, ( bank & BANK_MASK ) >> 5 );
        ENC28J60BANK = ( bank & BANK_MASK );
    }
}

u8 ENC28J60_Read ( u8 addr ) { /* ��ȡENC28J60ָ���Ĵ���������addr�ǼĴ�����ַ */
    ENC28J60_Set_Bank ( addr ); /* ����BANK */
    return ENC28J60_Read_Op ( ENC28J60_READ_CTRL_REG, addr );
}

void ENC28J60_Write ( u8 addr, u8 data ) { /* ��ENC28J60ָ���Ĵ���д���ݡ�����addr�ǼĴ�����ַ��data��Ҫд������� */
    ENC28J60_Set_Bank ( addr );
    ENC28J60_Write_Op ( ENC28J60_WRITE_CTRL_REG, addr, data );
}

void ENC28J60_PHY_Write ( u8 addr, u32 data ) { /* ��ENC28J60��PHY�Ĵ���д�����ݡ�����addr�ǼĴ�����ַ��data��Ҫд������� */
    u16 retry = 0;
    ENC28J60_Write ( MIREGADR, addr ); /* ����PHY�Ĵ�����ַ */
    ENC28J60_Write ( MIWRL, data ); /* д������ */
    ENC28J60_Write ( MIWRH, data >> 8 );

    while ( ( ENC28J60_Read ( MISTAT ) &MISTAT_BUSY ) && retry < 0XFFF ) {
        retry++; /* �ȴ�д��PHY���� */
    }
}

u8 ENC28J60_Init ( u8 *macaddr ) { /* ��ʼ��ENC28J60������macaddr��MAC��ַ */
    u16 retry = 0;
    ENC28J60_Reset();
    ENC28J60_Write_Op ( ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET ); /* �����λ */

    while ( ! ( ENC28J60_Read ( ESTAT ) &ESTAT_CLKRDY ) && retry < 500 ) { /* �ȴ�ʱ���ȶ� */
        retry++;
        delay_ms ( 1 );
    };

    if ( retry >= 500 ) {
        return 1; /* ENC28J60��ʼ��ʧ�� */
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

u8 ENC28J60_Get_EREVID ( void ) { /* ��ȡEREVID */
    return ENC28J60_Read ( EREVID );
}

#include "uip.h"
void ENC28J60_Packet_Send ( u32 len, u8 *packet ) { /* ͨ��ENC28J60�������ݰ������磬����len�����ݰ���С��packet�����ݰ� */
    /* ���÷��ͻ�������ַдָ����� */
    ENC28J60_Write ( EWRPTL, TXSTART_INIT & 0xFF );
    ENC28J60_Write ( EWRPTH, TXSTART_INIT >> 8 );
    /* ����TXNDָ�룬�Զ�Ӧ���������ݰ���С */
    ENC28J60_Write ( ETXNDL, ( TXSTART_INIT + len ) & 0xFF );
    ENC28J60_Write ( ETXNDH, ( TXSTART_INIT + len ) >> 8 );
    ENC28J60_Write_Op ( ENC28J60_WRITE_BUF_MEM, 0, 0x00 ); /* дÿ�������ֽ�(0x00��ʾʹ��macon3������) */
    ENC28J60_Write_Buf ( len, packet ); /* �������ݰ������ͻ����� */
    ENC28J60_Write_Op ( ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS ); /* �������ݵ����� */

    if ( ( ENC28J60_Read ( EIR ) &EIR_TXERIF ) ) {
        ENC28J60_Write_Op ( ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS );
    }
}

/* �������ȡһ�����ݰ����ݡ�����maxlen�����ݰ����������ճ��ȣ�packet�����ݰ������������������յ������ݰ�����(�ֽ�) */
u32 ENC28J60_Packet_Receive ( u32 maxlen, u8 *packet ) {
    u32 rxstat;
    u32 len;

    if ( ENC28J60_Read ( EPKTCNT ) == 0 ) {
        return 0;
    }

    /* ���ý��ջ�������ָ�� */
    ENC28J60_Write ( ERDPTL, ( NextPacketPtr ) );
    ENC28J60_Write ( ERDPTH, ( NextPacketPtr ) >> 8 );
    /* ����һ������ָ�� */
    NextPacketPtr = ENC28J60_Read_Op ( ENC28J60_READ_BUF_MEM, 0 );
    NextPacketPtr |= ENC28J60_Read_Op ( ENC28J60_READ_BUF_MEM, 0 ) << 8;
    /* �����ĳ��� */
    len = ENC28J60_Read_Op ( ENC28J60_READ_BUF_MEM, 0 );
    len |= ENC28J60_Read_Op ( ENC28J60_READ_BUF_MEM, 0 ) << 8;
    len -= 4; /* ȥ��CRC���� */
    /* ��ȡ����״̬ */
    rxstat = ENC28J60_Read_Op ( ENC28J60_READ_BUF_MEM, 0 );
    rxstat |= ENC28J60_Read_Op ( ENC28J60_READ_BUF_MEM, 0 ) << 8;

    if ( len > maxlen - 1 ) { /* ���ƽ��ճ��� */
        len = maxlen - 1;
    }

    if ( ( rxstat & 0x80 ) == 0 ) {
        len = 0;   
    } else {
        ENC28J60_Read_Buf ( len, packet ); /* �ӽ��ջ������и������ݰ� */
    }

    /* RX��ָ���ƶ�����һ�����յ������ݰ��Ŀ�ʼλ�ã����ͷ����ǸղŶ��������ڴ� */
    ENC28J60_Write ( ERXRDPTL, ( NextPacketPtr ) );
    ENC28J60_Write ( ERXRDPTH, ( NextPacketPtr ) >> 8 );
    ENC28J60_Write_Op ( ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC );
    return ( len );
}