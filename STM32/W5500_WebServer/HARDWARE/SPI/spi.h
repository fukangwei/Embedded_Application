#ifndef __SPI_H
#define __SPI_H
#include "sys.h"

#define WIZ_SCS  GPIO_Pin_4 /* out */
#define WIZ_SCLK GPIO_Pin_5 /* out */
#define WIZ_MISO GPIO_Pin_6 /* in */
#define WIZ_MOSI GPIO_Pin_7 /* out */

void WIZ_CS ( uint8_t val );
void SPIx_Init ( void );
void SPIx_SetSpeed ( u8 SpeedSet );
u8 SPIx_ReadWriteByte ( u8 TxData );
#endif
