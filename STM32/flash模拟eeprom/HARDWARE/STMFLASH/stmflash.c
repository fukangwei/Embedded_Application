#include "stmflash.h"
#include "delay.h"
#include "usart.h"

u16 STMFLASH_ReadHalfWord ( u32 faddr ) { /* ��ȡָ����ַ�İ���(16λ����)������faddr�Ƕ���ַ(�˵�ַ����Ϊ2�ı���) */
    return * ( vu16 * ) faddr;
}

#if STM32_FLASH_WREN  /* ���ʹ����д */

/* ������д�롣����WriteAddr����ʼ��ַ��pBuffer������ָ�룬NumToWrite�ǰ���(16λ)������ */
void STMFLASH_Write_NoCheck ( u32 WriteAddr, u16 *pBuffer, u16 NumToWrite ) {
    u16 i;

    for ( i = 0; i < NumToWrite; i++ ) {
        FLASH_ProgramHalfWord ( WriteAddr, pBuffer[i] );
        WriteAddr += 2; /* ��ַ����2 */
    }
}

#if STM32_FLASH_SIZE < 256
    #define STM_SECTOR_SIZE 1024 /* �ֽ� */
#else
    #define STM_SECTOR_SIZE 2048
#endif

u16 STMFLASH_BUF[STM_SECTOR_SIZE / 2]; /* �����2K�ֽ� */

void STMFLASH_Write ( u32 WriteAddr, u16 *pBuffer, u16 NumToWrite ) { /* ��ָ����ַ��ʼд��ָ�����ȵ����� */
    u32 secpos; /* ������ַ */
    u16 secoff; /* ������ƫ�Ƶ�ַ(16λ�ּ���) */
    u16 secremain; /* ������ʣ���ַ(16λ�ּ���) */
    u16 i;
    u32 offaddr; /* ȥ��0X08000000��ĵ�ַ */

    if ( WriteAddr < STM32_FLASH_BASE || ( WriteAddr >= ( STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE ) ) ) { /* �Ƿ���ַ */
        return;
    }

    FLASH_Unlock(); /* ���� */
    offaddr = WriteAddr - STM32_FLASH_BASE; /* ʵ��ƫ�Ƶ�ַ */
    secpos = offaddr / STM_SECTOR_SIZE; /* ������ַ */
    secoff = ( offaddr % STM_SECTOR_SIZE ) / 2; /* �������ڵ�ƫ��(2���ֽ�Ϊ������λ) */
    secremain = STM_SECTOR_SIZE / 2 - secoff; /* ����ʣ��ռ��С */

    if ( NumToWrite <= secremain ) { /* �����ڸ�������Χ */
        secremain = NumToWrite;
    }

    while ( 1 ) {
        STMFLASH_Read ( secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2 ); /* ������������������ */

        for ( i = 0; i < secremain; i++ ) { /* У������ */
            if ( STMFLASH_BUF[secoff + i] != 0XFFFF ) { /* ������Ҫ���� */
                break;
            }
        }

        if ( i < secremain ) { /* �����Ҫ���� */
            FLASH_ErasePage ( secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE ); /* ����������� */

            for ( i = 0; i < secremain; i++ ) { /* ���� */
                STMFLASH_BUF[i + secoff] = pBuffer[i];
            }

            STMFLASH_Write_NoCheck ( secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2 ); /* д���������� */
        } else {
            STMFLASH_Write_NoCheck ( WriteAddr, pBuffer, secremain );
        }

        if ( NumToWrite == secremain ) {
            break; /* д������� */
        } else { /* д��δ���� */
            secpos++; /* ������ַ��1 */
            secoff = 0; /* ƫ��λ��Ϊ0 */
            pBuffer += secremain; /* ָ��ƫ�� */
            WriteAddr += secremain; /* д��ַƫ�� */
            NumToWrite -= secremain; /* �ֽ�(16λ)���ݼ� */

            if ( NumToWrite > ( STM_SECTOR_SIZE / 2 ) ) {
                secremain = STM_SECTOR_SIZE / 2; /* ��һ����������д���� */
            } else {
                secremain = NumToWrite; /* ��һ����������д���� */
            }
        }
    };

    FLASH_Lock(); /* ���� */
}
#endif

void STMFLASH_Read ( u32 ReadAddr, u16 *pBuffer, u16 NumToRead ) { /* ��ָ����ַ��ʼ����ָ�����ȵ����� */
    u16 i;

    for ( i = 0; i < NumToRead; i++ ) {
        pBuffer[i] = STMFLASH_ReadHalfWord ( ReadAddr ); /* ��ȡ2���ֽ� */
        ReadAddr += 2; /* ƫ��2���ֽ� */
    }
}

void Test_Write ( u32 WriteAddr, u16 WriteData ) {
    STMFLASH_Write ( WriteAddr, &WriteData, 1 );
}