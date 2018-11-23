#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "key.h"
#include "dma.h"

const u8 TEXT_TO_SEND[] = {"ALIENTEK Mini STM32 DMA 串口实验"};
#define TEXT_LENTH sizeof(TEXT_TO_SEND)-1 /* TEXT_TO_SEND字符串长度(不包含结束符) */
u8 SendBuff[ ( TEXT_LENTH + 2 ) * 100];

int main ( void ) {
    u16 i;
    u8 t = 0;
    float pro = 0; /* 进度 */
    delay_init();
    uart_init ( 9600 );
    LED_Init();
    LCD_Init();
    KEY_Init();
    /* DMA1通道4，外设为串口1，存储器为SendBuff，长为“(TEXT_LENTH + 2) * 100” */
    MYDMA_Config ( DMA1_Channel4, ( u32 ) &USART1->DR, ( u32 ) SendBuff, ( TEXT_LENTH + 2 ) * 100 );
    POINT_COLOR = RED;
    LCD_ShowString ( 60, 50, 200, 16, 16, "Mini STM32" );
    LCD_ShowString ( 60, 70, 200, 16, 16, "DMA TEST" );

    for ( i = 0; i < ( TEXT_LENTH + 2 ) * 100; i++ ) {
        if ( t >= TEXT_LENTH ) { /* 加入换行符 */
            SendBuff[i++] = 0x0d;
            SendBuff[i] = 0x0a;
            t = 0;
        } else {
            SendBuff[i] = TEXT_TO_SEND[t++]; /* 复制TEXT_TO_SEND语句 */
        }
    }

    POINT_COLOR = BLUE;
    i = 0;

    while ( 1 ) {
        t = KEY_Scan();

        if ( t == KEY0 ) { /* KEY0按下 */
            LCD_ShowString ( 60, 150, 200, 16, 16, "Start Transimit...." );
            LCD_ShowString ( 60, 170, 200, 16, 16, "   %" );
            printf ( "\r\nDMA DATA:\r\n " );
            USART_DMACmd ( USART1, USART_DMAReq_Tx, ENABLE ); /* 使能串口1的DMA发送 */
            MYDMA_Enable ( DMA1_Channel4 ); //开始一次DMA传输

            while ( 1 ) { /*等待DMA传输完成，此时来做另外一些事，例如点灯。在实际应用中，传输数据期间可以执行另外的任务*/
                if ( DMA_GetFlagStatus ( DMA1_FLAG_TC4 ) != RESET ) { /* 等待通道4传输完成 */
                    DMA_ClearFlag ( DMA1_FLAG_TC4 ); /* 清除通道4传输完成标志 */
                    break;
                }

                pro = DMA_GetCurrDataCounter ( DMA1_Channel4 ); /* 得到当前还剩余多少个数据 */
                pro = 1 - pro / ( ( TEXT_LENTH + 2 ) * 100 ); /* 得到百分比 */
                pro *= 100;
                LCD_ShowNum ( 60, 170, pro, 3, 16 );
            }

            LCD_ShowNum ( 60, 170, 100, 3, 16 );
            LCD_ShowString ( 60, 150, 200, 16, 16, "Transimit Finished!" );
        }

        i++;
        delay_ms ( 10 );

        if ( i == 20 ) {
            LED0 = !LED0;
            i = 0;
        }
    }
}