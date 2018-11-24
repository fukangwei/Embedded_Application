#ifndef  _W5500_H_
#define  _W5500_H_
#include "Types.h"
#include "sys.h"

#define MR        (0x000000)
#define GAR0      (0x000100)
#define GAR1      (0x000200)
#define GAR2      (0x000300)
#define GAR3      (0x000400)
#define SUBR0     (0x000500)
#define SUBR1     (0x000600)
#define SUBR2     (0x000700)
#define SUBR3     (0x000800)
#define SHAR0     (0x000900)
#define SHAR1     (0x000A00)
#define SHAR2     (0x000B00)
#define SHAR3     (0x000C00)
#define SHAR4     (0x000D00)
#define SHAR5     (0x000E00)
#define SIPR0     (0x000F00)
#define SIPR1     (0x001000)
#define SIPR2     (0x001100)
#define SIPR3     (0x001200)
#define INTLEVEL0 (0x001300)
#define INTLEVEL1 (0x001400)
#define IR        (0x001500)
#define IMR       (0x001600)
#define SIR       (0x001700)
#define SIMR      (0x001800)
#define RTR0      (0x001900)
#define RTR1      (0x001A00)
#define WIZ_RCR   (0x001B00)
#define PTIMER    (0x001C00)
#define PMAGIC    (0x001D00)
#define PDHAR0    (0x001E00)
#define PDHAR1    (0x001F00)
#define PDHAR2    (0x002000)
#define PDHAR3    (0x002100)
#define PDHAR4    (0x002200)
#define PDHAR5    (0x002300)
#define PSID0     (0x002400)
#define PSID1     (0x002500)
#define PMR0      (0x002600)
#define PMR1      (0x002700)
#define UIPR0     (0x002800)
#define UIPR1     (0x002900)
#define UIPR2     (0x002A00)
#define UIPR3     (0x002B00)
#define UPORT0    (0x002C00)
#define UPORT1    (0x002D00)
#define PHYCFGR   (0x002E00)
#define VERSIONR  (0x003900)

#define Sn_MR(ch)         (0x000008 + (ch << 5))
#define Sn_CR(ch)         (0x000108 + (ch << 5))
#define Sn_IR(ch)         (0x000208 + (ch << 5))
#define Sn_SR(ch)         (0x000308 + (ch << 5))
#define Sn_PORT0(ch)      (0x000408 + (ch << 5))
#define Sn_PORT1(ch)      (0x000508 + (ch << 5))
#define Sn_DHAR0(ch)      (0x000608 + (ch << 5))
#define Sn_DHAR1(ch)      (0x000708 + (ch << 5))
#define Sn_DHAR2(ch)      (0x000808 + (ch << 5))
#define Sn_DHAR3(ch)      (0x000908 + (ch << 5))
#define Sn_DHAR4(ch)      (0x000A08 + (ch << 5))
#define Sn_DHAR5(ch)      (0x000B08 + (ch << 5))
#define Sn_DIPR0(ch)      (0x000C08 + (ch << 5))
#define Sn_DIPR1(ch)      (0x000D08 + (ch << 5))
#define Sn_DIPR2(ch)      (0x000E08 + (ch << 5))
#define Sn_DIPR3(ch)      (0x000F08 + (ch << 5))
#define Sn_DPORT0(ch)     (0x001008 + (ch << 5))
#define Sn_DPORT1(ch)     (0x001108 + (ch << 5))
#define Sn_MSSR0(ch)      (0x001208 + (ch << 5))
#define Sn_MSSR1(ch)      (0x001308 + (ch << 5))
#define Sn_TOS(ch)        (0x001508 + (ch << 5))
#define Sn_TTL(ch)        (0x001608 + (ch << 5))
#define Sn_RXMEM_SIZE(ch) (0x001E08 + (ch << 5))
#define Sn_TXMEM_SIZE(ch) (0x001F08 + (ch << 5))
#define Sn_TX_FSR0(ch)    (0x002008 + (ch << 5))
#define Sn_TX_FSR1(ch)    (0x002108 + (ch << 5))
#define Sn_TX_RD0(ch)     (0x002208 + (ch << 5))
#define Sn_TX_RD1(ch)     (0x002308 + (ch << 5))
#define Sn_TX_WR0(ch)     (0x002408 + (ch << 5))
#define Sn_TX_WR1(ch)     (0x002508 + (ch << 5))
#define Sn_RX_RSR0(ch)    (0x002608 + (ch << 5))
#define Sn_RX_RSR1(ch)    (0x002708 + (ch << 5))
#define Sn_RX_RD0(ch)     (0x002808 + (ch << 5))
#define Sn_RX_RD1(ch)     (0x002908 + (ch << 5))
#define Sn_RX_WR0(ch)     (0x002A08 + (ch << 5))
#define Sn_RX_WR1(ch)     (0x002B08 + (ch << 5))
#define Sn_IMR(ch)        (0x002C08 + (ch << 5))
#define Sn_FRAG(ch)       (0x002D08 + (ch << 5))
#define Sn_KPALVTR(ch)    (0x002F08 + (ch << 5))

#define MR_RST      0x80
#define MR_WOL      0x20
#define MR_PB       0x10
#define MR_PPPOE    0x08
#define MR_UDP_FARP 0x02
#define IR_CONFLICT 0x80
#define IR_UNREACH  0x40
#define IR_PPPoE    0x20
#define IR_MAGIC    0x10

#define Sn_MR_CLOSE  0x00
#define Sn_MR_TCP    0x01
#define Sn_MR_UDP    0x02
#define Sn_MR_IPRAW  0x03
#define Sn_MR_MACRAW 0x04
#define Sn_MR_PPPOE  0x05
#define Sn_MR_UCASTB 0x10
#define Sn_MR_ND     0x20
#define Sn_MR_MC     0x20
#define Sn_MR_BCASTB 0x40
#define Sn_MR_MULTI  0x80

#define Sn_MR_MIP6N 0x10
#define Sn_MR_MMB   0x20
#define Sn_MR_MFEN  0x80

#define Sn_CR_OPEN      0x01
#define Sn_CR_LISTEN    0x02
#define Sn_CR_CONNECT   0x04
#define Sn_CR_DISCON    0x08
#define Sn_CR_CLOSE     0x10
#define Sn_CR_SEND      0x20
#define Sn_CR_SEND_MAC  0x21
#define Sn_CR_SEND_KEEP 0x22
#define Sn_CR_RECV      0x40

#ifdef __DEF_IINCHIP_PPP__
    #define Sn_CR_PCON    0x23
    #define Sn_CR_PDISCON 0x24
    #define Sn_CR_PCR     0x25
    #define Sn_CR_PCN     0x26
    #define Sn_CR_PCJ     0x27
#endif

#ifdef __DEF_IINCHIP_PPP__
    #define Sn_IR_PRECV 0x80
    #define Sn_IR_PFAIL 0x40
    #define Sn_IR_PNEXT 0x20
#endif

#define Sn_IR_SEND_OK 0x10
#define Sn_IR_TIMEOUT 0x08
#define Sn_IR_RECV    0x04
#define Sn_IR_DISCON  0x02
#define Sn_IR_CON     0x01

#define SOCK_CLOSED      0x00
#define SOCK_INIT        0x13
#define SOCK_LISTEN      0x14
#define SOCK_SYNSENT     0x15
#define SOCK_SYNRECV     0x16
#define SOCK_ESTABLISHED 0x17
#define SOCK_FIN_WAIT    0x18
#define SOCK_CLOSING     0x1A
#define SOCK_TIME_WAIT   0x1B
#define SOCK_CLOSE_WAIT  0x1C
#define SOCK_LAST_ACK    0x1D
#define SOCK_UDP         0x22
#define SOCK_IPRAW       0x32
#define SOCK_MACRAW      0x42
#define SOCK_PPPOE       0x5F

#define IPPROTO_IP   0
#define IPPROTO_ICMP 1
#define IPPROTO_IGMP 2
#define IPPROTO_GGP  3
#define IPPROTO_TCP  6
#define IPPROTO_PUP  12
#define IPPROTO_UDP  17
#define IPPROTO_IDP  22
#define IPPROTO_ND   77
#define IPPROTO_RAW  255

void IINCHIP_WRITE ( uint32 addrbsb,  uint8 data );
uint8 IINCHIP_READ ( uint32 addrbsb );
uint16 wiz_write_buf ( uint32 addrbsb, uint8 *buf, uint16 len );
uint16 wiz_read_buf ( uint32 addrbsb, uint8 *buf, uint16 len );
void iinchip_init ( void );
void sysinit ( uint8 *tx_size, uint8 *rx_size );
uint8 getISR ( uint8 s );
void putISR ( uint8 s, uint8 val );
uint16 getIINCHIP_RxMAX ( uint8 s );
uint16 getIINCHIP_TxMAX ( uint8 s );
void setMR ( uint8 val );
void setRTR ( uint16 timeout );
void setRCR ( uint8 retry );
void clearIR ( uint8 mask );
uint8 getIR ( void );
void setSn_MSS ( SOCKET s, uint16 Sn_MSSR );
uint8 getSn_IR ( SOCKET s );
uint8 getSn_SR ( SOCKET s );
uint16 getSn_TX_FSR ( SOCKET s );
uint16 getSn_RX_RSR ( SOCKET s );
uint8 getSn_SR ( SOCKET s );
void setSn_TTL ( SOCKET s, uint8 ttl );
void send_data_processing ( SOCKET s, uint8 *wizdata, uint16 len );
void recv_data_processing ( SOCKET s, uint8 *wizdata, uint16 len );
void setGAR ( uint8 *addr );
void setSUBR ( uint8 *addr );
void setSHAR ( uint8 *addr );
void setSIPR ( uint8 *addr );
void getGAR ( uint8 *addr );
void getSUBR ( uint8 *addr );
void getSHAR ( uint8 *addr );
void getSIPR ( uint8 *addr );
void setSn_IR ( uint8 s, uint8 val );
#endif