#include "mmc_sd.h"
#include "diskio.h"
#include "malloc.h"
#include  <stdio.h>

#define SD_CARD  0 /* SD卡，卷标为0 */
#define EX_FLASH 1 /* 外部flash，卷标为1 */

#define FLASH_SECTOR_SIZE 512
/* 前2M字节给fatfs用，后2M字节至“2M+500K”给用户用，“2M+500K”以后用于存放字库，字库占用1.5M */
#define FLASH_SECTOR_COUNT 4096 /* 总共有4096个扇区(以512字节算)，W25Q16为2048，Q32为4096，Q64为16384，这里强制用512字节 */
#define FLASH_BLOCK_SIZE 8 /* 每个BLOCK有8个扇区 */

DSTATUS disk_initialize ( /* 初始化磁盘 */
    BYTE drv /* Physical drive nmuber (0..) */
) {
    u8 res;

    switch ( drv ) {
        case SD_CARD: /* SD卡 */
            res = SD_Initialize();
            break;

        default:
            res = 1;
    }

    if ( res ) {
        return  STA_NOINIT;
    } else {
        return 0; /* 初始化成功 */
    }
}

DSTATUS disk_status ( /* 获得磁盘状态 */
    BYTE drv /* Physical drive nmuber (0..) */
) {
    return 0;
}

DRESULT disk_read ( /* 读扇区，参数drv是磁盘编号(0~9)，buff是数据接收缓冲首地址，sector是扇区地址，count是需要读取的扇区数 */
    BYTE drv, /* Physical drive nmuber (0..) */
    BYTE *buff, /* Data buffer to store read data */
    DWORD sector, /* Sector address (LBA) */
    BYTE count /* Number of sectors to read (1..255) */
) {
    u8 res = 0;

    if ( !count ) {
        return RES_PARERR; /* count不能等于0 */
    }

    switch ( drv ) {
        case SD_CARD: /* SD卡 */
            res = SD_ReadDisk ( buff, sector, count );
            break;

        default:
            res = 1;
    }

    if ( res == 0x00 ) { /* 处理返回值，将SPI_SD_driver.c的返回值转成ff.c的返回值 */
        return RES_OK;
    } else {
        return RES_ERROR;
    }
}

#if _READONLY == 0
DRESULT disk_write ( /* 写扇区，参数drv是磁盘编号(0~9)，buff是发送数据首地址，sector是扇区地址，count是需要写入的扇区数 */
    BYTE drv, /* Physical drive nmuber (0..) */
    const BYTE *buff, /* Data to be written */
    DWORD sector, /* Sector address (LBA) */
    BYTE count /* Number of sectors to write (1..255) */
) {
    u8 res = 0;

    if ( !count ) {
        return RES_PARERR; /* count不能等于0 */
    }

    switch ( drv ) {
        case SD_CARD://SD卡
            res = SD_WriteDisk ( ( u8 * ) buff, sector, count );
            break;

        default:
            res = 1;
    }

    if ( res == 0x00 ) { /* 处理返回值，将SPI_SD_driver.c的返回值转成ff.c的返回值 */
        return RES_OK;
    } else {
        return RES_ERROR;
    }
}
#endif /* _READONLY */

DRESULT disk_ioctl (
    BYTE drv, /* Physical drive nmuber (0..) */
    BYTE ctrl, /* Control code */
    void *buff /* Buffer to send/receive control data */
) {
    DRESULT res;

    if ( drv == SD_CARD ) { /* SD卡 */
        switch ( ctrl ) {
            case CTRL_SYNC:
                SD_CS = 0;

                if ( SD_WaitReady() == 0 ) {
                    res = RES_OK;
                } else {
                    res = RES_ERROR;
                }

                SD_CS = 1;
                break;

            case GET_SECTOR_SIZE:
                * ( WORD * ) buff = 512;
                res = RES_OK;
                break;

            case GET_BLOCK_SIZE:
                * ( WORD * ) buff = 8;
                res = RES_OK;
                break;

            case GET_SECTOR_COUNT:
                * ( DWORD * ) buff = SD_GetSectorCount();
                res = RES_OK;
                break;

            default:
                res = RES_PARERR;
                break;
        }
    } else {
        res = RES_ERROR;
    }

    return res;
}

/* User defined function to give a current time to fatfs module
   31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31)
   15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */
DWORD get_fattime ( void ) {
    return 0;
}

void *ff_memalloc ( UINT size ) {
    return ( void * ) mymalloc ( size );
}

void ff_memfree ( void *mf ) {
    myfree ( mf );
}