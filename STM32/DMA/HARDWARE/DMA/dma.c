#include "dma.h"

DMA_InitTypeDef DMA_InitStructure;

u16 DMA1_MEM_LEN; /* 保存DMA每次数据传送的长度 */

/* DMA1的各通道配置。参数DMA_CHx是DMA通道CHx，cpar是外设地址，cmar是存储器地址，cndtr是数据传输量 */
void MYDMA_Config ( DMA_Channel_TypeDef *DMA_CHx, u32 cpar, u32 cmar, u16 cndtr ) {
    RCC_AHBPeriphClockCmd ( RCC_AHBPeriph_DMA1, ENABLE ); /* 使能DMA传输 */
    DMA_DeInit ( DMA_CHx ); /* 将DMA的通道1寄存器重设为缺省值 */
    DMA1_MEM_LEN = cndtr;
    DMA_InitStructure.DMA_PeripheralBaseAddr = cpar; /* DMA外设ADC基地址 */
    DMA_InitStructure.DMA_MemoryBaseAddr = cmar; /* DMA内存基地址 */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; /* 数据传输方向，从内存读取发送到外设 */
    DMA_InitStructure.DMA_BufferSize = cndtr; /* DMA通道的DMA缓存的大小 */
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; /* 外设地址寄存器不变 */
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; /* 内存地址寄存器递增 */
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; /* 数据宽度为8位 */
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; /* 数据宽度为8位 */
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; /* 工作在正常缓存模式 */
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; /* DMA通道x拥有中优先级 */
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; /* DMA通道x没有设置为内存到内存传输 */
    DMA_Init ( DMA_CHx, &DMA_InitStructure ); /* 初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器 */
}

void MYDMA_Enable ( DMA_Channel_TypeDef *DMA_CHx ) { /* 开启一次DMA传输 */
    DMA_Cmd ( DMA_CHx, DISABLE ); /* 关闭“USART1 TX DMA1”所指示的通道 */
    DMA_SetCurrDataCounter ( DMA1_Channel4, DMA1_MEM_LEN ); /* DMA通道的DMA缓存的大小 */
    DMA_Cmd ( DMA_CHx, ENABLE ); /* 使能“USART1 TX DMA1”所指示的通道 */
}