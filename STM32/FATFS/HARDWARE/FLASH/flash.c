#include "flash.h"
#include "spi.h"
#include "delay.h"

void SPI_Flash_Init ( void ) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    GPIO_SetBits ( GPIOA, GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 );
    SPIx_Init();
}

u8 SPI_Flash_ReadSR ( void ) {
    u8 byte = 0;
    SPI_FLASH_CS = 0;
    SPIx_ReadWriteByte ( W25X_ReadStatusReg );
    byte = SPIx_ReadWriteByte ( 0Xff );
    SPI_FLASH_CS = 1;
    return byte;
}

void SPI_FLASH_Write_SR ( u8 sr ) {
    SPI_FLASH_CS = 0;
    SPIx_ReadWriteByte ( W25X_WriteStatusReg );
    SPIx_ReadWriteByte ( sr );
    SPI_FLASH_CS = 1;
}

void SPI_FLASH_Write_Enable ( void ) {
    SPI_FLASH_CS = 0;
    SPIx_ReadWriteByte ( W25X_WriteEnable );
    SPI_FLASH_CS = 1;
}

void SPI_FLASH_Write_Disable ( void ) {
    SPI_FLASH_CS = 0;
    SPIx_ReadWriteByte ( W25X_WriteDisable );
    SPI_FLASH_CS = 1;
}

u16 SPI_Flash_ReadID ( void ) {
    u16 Temp = 0;
    SPI_FLASH_CS = 0;
    SPIx_ReadWriteByte ( 0x90 );
    SPIx_ReadWriteByte ( 0x00 );
    SPIx_ReadWriteByte ( 0x00 );
    SPIx_ReadWriteByte ( 0x00 );
    Temp |= SPIx_ReadWriteByte ( 0xFF ) << 8;
    Temp |= SPIx_ReadWriteByte ( 0xFF );
    SPI_FLASH_CS = 1;
    return Temp;
}

void SPI_Flash_Read ( u8 *pBuffer, u32 ReadAddr, u16 NumByteToRead ) {
    u16 i;
    SPI_FLASH_CS = 0;
    SPIx_ReadWriteByte ( W25X_ReadData );
    SPIx_ReadWriteByte ( ( u8 ) ( ( ReadAddr ) >> 16 ) );
    SPIx_ReadWriteByte ( ( u8 ) ( ( ReadAddr ) >> 8 ) );
    SPIx_ReadWriteByte ( ( u8 ) ReadAddr );

    for ( i = 0; i < NumByteToRead; i++ ) {
        pBuffer[i] = SPIx_ReadWriteByte ( 0XFF );
    }

    SPI_FLASH_CS = 1;
}

void SPI_Flash_Write_Page ( u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite ) {
    u16 i;
    SPI_FLASH_Write_Enable();
    SPI_FLASH_CS = 0;
    SPIx_ReadWriteByte ( W25X_PageProgram );
    SPIx_ReadWriteByte ( ( u8 ) ( ( WriteAddr ) >> 16 ) );
    SPIx_ReadWriteByte ( ( u8 ) ( ( WriteAddr ) >> 8 ) );
    SPIx_ReadWriteByte ( ( u8 ) WriteAddr );

    for ( i = 0; i < NumByteToWrite; i++ ) {
        SPIx_ReadWriteByte ( pBuffer[i] );
    }

    SPI_FLASH_CS = 1;
    SPI_Flash_Wait_Busy();
}

void SPI_Flash_Write_NoCheck ( u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite ) {
    u16 pageremain;
    pageremain = 256 - WriteAddr % 256;

    if ( NumByteToWrite <= pageremain ) {
        pageremain = NumByteToWrite;
    }

    while ( 1 ) {
        SPI_Flash_Write_Page ( pBuffer, WriteAddr, pageremain );

        if ( NumByteToWrite == pageremain ) {
            break;
        } else {
            pBuffer += pageremain;
            WriteAddr += pageremain;
            NumByteToWrite -= pageremain;

            if ( NumByteToWrite > 256 ) {
                pageremain = 256;
            } else {
                pageremain = NumByteToWrite;
            }
        }
    };
}

u8 SPI_FLASH_BUF[4096];
void SPI_Flash_Write ( u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite ) {
    u32 secpos;
    u16 secoff;
    u16 secremain;
    u16 i;
    secpos = WriteAddr / 4096;
    secoff = WriteAddr % 4096;
    secremain = 4096 - secoff;

    if ( NumByteToWrite <= secremain ) {
        secremain = NumByteToWrite;
    }

    while ( 1 ) {
        SPI_Flash_Read ( SPI_FLASH_BUF, secpos * 4096, 4096 );

        for ( i = 0; i < secremain; i++ ) {
            if ( SPI_FLASH_BUF[secoff + i] != 0XFF ) {
                break;
            }
        }

        if ( i < secremain ) {
            SPI_Flash_Erase_Sector ( secpos );

            for ( i = 0; i < secremain; i++ ) {
                SPI_FLASH_BUF[i + secoff] = pBuffer[i];
            }

            SPI_Flash_Write_NoCheck ( SPI_FLASH_BUF, secpos * 4096, 4096 );
        } else {
            SPI_Flash_Write_NoCheck ( pBuffer, WriteAddr, secremain );
        }

        if ( NumByteToWrite == secremain ) {
            break;
        } else {
            secpos++;
            secoff = 0;
            pBuffer += secremain;
            WriteAddr += secremain;
            NumByteToWrite -= secremain;

            if ( NumByteToWrite > 4096 ) {
                secremain = 4096;
            } else {
                secremain = NumByteToWrite;
            }
        }
    };
}

void SPI_Flash_Erase_Chip ( void ) {
    SPI_FLASH_Write_Enable();
    SPI_Flash_Wait_Busy();
    SPI_FLASH_CS = 0;
    SPIx_ReadWriteByte ( W25X_ChipErase );
    SPI_FLASH_CS = 1;
    SPI_Flash_Wait_Busy();
}

void SPI_Flash_Erase_Sector ( u32 Dst_Addr ) {
    Dst_Addr *= 4096;
    SPI_FLASH_Write_Enable();
    SPI_Flash_Wait_Busy();
    SPI_FLASH_CS = 0;
    SPIx_ReadWriteByte ( W25X_SectorErase );
    SPIx_ReadWriteByte ( ( u8 ) ( ( Dst_Addr ) >> 16 ) );
    SPIx_ReadWriteByte ( ( u8 ) ( ( Dst_Addr ) >> 8 ) );
    SPIx_ReadWriteByte ( ( u8 ) Dst_Addr );
    SPI_FLASH_CS = 1;
    SPI_Flash_Wait_Busy();
}

void SPI_Flash_Wait_Busy ( void ) {
    while ( ( SPI_Flash_ReadSR() & 0x01 ) == 0x01 );
}

void SPI_Flash_PowerDown ( void ) {
    SPI_FLASH_CS = 0;
    SPIx_ReadWriteByte ( W25X_PowerDown );
    SPI_FLASH_CS = 1;
    delay_us ( 3 );
}

void SPI_Flash_WAKEUP ( void ) {
    SPI_FLASH_CS = 0;
    SPIx_ReadWriteByte ( W25X_ReleasePowerDown );
    SPI_FLASH_CS = 1;
    delay_us ( 3 );
}