#ifndef _MMC_SD_H_
#define _MMC_SD_H_
#include "sys.h"

#define SD_TYPE_ERR  0X00
#define SD_TYPE_MMC  0X01
#define SD_TYPE_V1   0X02
#define SD_TYPE_V2   0X04
#define SD_TYPE_V2HC 0X06

#define CMD0  0
#define CMD1  1
#define CMD8  8
#define CMD9  9
#define CMD10 10
#define CMD12 12
#define CMD16 16
#define CMD17 17
#define CMD18 18
#define CMD23 23
#define CMD24 24
#define CMD25 25
#define CMD41 41
#define CMD55 55
#define CMD58 58
#define CMD59 59

#define MSD_DATA_OK              0x05
#define MSD_DATA_CRC_ERROR       0x0B
#define MSD_DATA_WRITE_ERROR     0x0D
#define MSD_DATA_OTHER_ERROR     0xFF
#define MSD_RESPONSE_NO_ERROR    0x00
#define MSD_IN_IDLE_STATE        0x01
#define MSD_ERASE_RESET          0x02
#define MSD_ILLEGAL_COMMAND      0x04
#define MSD_COM_CRC_ERROR        0x08
#define MSD_ERASE_SEQUENCE_ERROR 0x10
#define MSD_ADDRESS_ERROR        0x20
#define MSD_PARAMETER_ERROR      0x40
#define MSD_RESPONSE_FAILURE     0xFF

#define SD_CS PAout(3)

extern u8 SD_Type;

u8 SD_WaitReady ( void );
u8 SD_GetResponse ( u8 Response );
u8 SD_Initialize ( void );
u8 SD_ReadDisk ( u8 *buf, u32 sector, u8 cnt );
u8 SD_WriteDisk ( u8 *buf, u32 sector, u8 cnt );
u32 SD_GetSectorCount ( void );
u8 SD_GetCID ( u8 *cid_data );
u8 SD_GetCSD ( u8 *csd_data );
#endif