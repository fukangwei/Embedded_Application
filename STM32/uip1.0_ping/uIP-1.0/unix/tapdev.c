#include "tapdev.h"
#include "uip.h"
#include "enc28j60.h"

extern struct uip_eth_addr uip_ethaddr;

const u8 mymac[6] = {0x04, 0x02, 0x35, 0x00, 0x00, 0x01};

void tapdev_init ( void ) {
    u8 i;
    ENC28J60_Init ( ( u8 * ) mymac );

    for ( i = 0; i < 6; i++ ) {
        uip_ethaddr.addr[i] = mymac[i];
    }

    ENC28J60_PHY_Write ( PHLCON, 0x0476 );
}

unsigned int tapdev_read ( void ) {
    return  ENC28J60_Packet_Receive ( MAX_FRAMELEN, uip_buf );
}

void tapdev_send ( void ) {
    ENC28J60_Packet_Send ( uip_len, uip_buf );
}