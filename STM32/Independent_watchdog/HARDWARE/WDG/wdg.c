#include "wdg.h"
#include "led.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_wwdg.h"

void IWDG_Init ( u8 prer, u16 rlr ) {
    IWDG_WriteAccessCmd ( IWDG_WriteAccess_Enable ); /* 使能对寄存器IWDG_PR和IWDG_RLR的写操作 */
    IWDG_SetPrescaler ( prer ); /* 设置IWDG预分频值为64 */
    IWDG_SetReload ( rlr ); /* 设置IWDG重装载值 */
    IWDG_ReloadCounter(); /* 按照IWDG重装载寄存器的值重装载IWDG计数器 */
    IWDG_Enable(); /* 使能IWDG */
}

void IWDG_Feed ( void ) { /* 喂独立看门狗 */
    IWDG_ReloadCounter();
}

void WWDG_Init ( u8 tr, u8 wr, u32 fprer ) {
    RCC_APB1PeriphClockCmd ( RCC_APB1Periph_WWDG, ENABLE ); /* WWDG时钟使能 */
    WWDG_SetPrescaler ( fprer ); /* 设置IWDG预分频值 */
    WWDG_SetWindowValue ( wr ); /* 设置窗口值 */
    WWDG_Enable ( tr ); /* 使能看门狗 */
    WWDG_ClearFlag();
    WWDG_NVIC_Init(); /* 初始化窗口看门狗NVIC */
    WWDG_EnableIT(); /* 开启窗口看门狗中断 */
}

void WWDG_Set_Counter ( u8 cnt ) { /* 重设置WWDG计数器的值 */
    WWDG_Enable ( cnt );
}

void WWDG_NVIC_Init() { /* 窗口看门狗中断服务程序 */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn; /* WWDG中断 */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_Init ( &NVIC_InitStructure ); /* NVIC初始化 */
}

void WWDG_IRQHandler ( void ) {
    WWDG_SetCounter ( 0x7F ); /* 如果禁掉此句，窗口看门狗将产生复位 */
    WWDG_ClearFlag(); /* 清除提前唤醒中断标志位 */
}