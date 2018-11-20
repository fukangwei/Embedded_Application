#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "adc.h"
#include "stdio.h"
#include "timerx.h"
#include "stm32_dsp.h"
#include "math.h"

#define NPT 64 /* FFT采样点数 */
#define RED_SUB_SPEED  60 /* 红色频柱向下缩短速度，值越大速度越慢 */
#define GREEN_SUB_SPEED 100 /* 绿色点下移速度，值越大速度越慢 */
#define GREEN_STOP_TIME 10 /* 绿色点顶端停顿时间，值越大时间越长 */

#define DISABLE_JTAG do \
    { \
        RCC->APB2ENR |= 0x00000001; /* 开启afio时钟 */ \
        AFIO->MAPR = ( 0x00FFFFFF & AFIO->MAPR ) | 0x04000000; /* 关闭JTAG */ \
    }while(0);

extern u32 ADC_ConvertedValue;
extern volatile uint8_t ADC_TimeOutFlag;

uint32_t ADC_DataNum = 0; /* ADC当前采样点数 */
unsigned long lBUFOUT[NPT] = {0};
unsigned long lBUFIN[NPT] = {0};
unsigned long lBUFMAG[NPT + NPT / 2]; /* 存储求模后的数据 */
uint16_t fftHightRedBuf[NPT / 2] = {0}; /* 红色频柱高度数组 */
uint32_t RedTime = 0; /* 红色点下移时间变量 */
uint16_t fftHightGreenBuf[NPT / 2] = {0}; /* 绿色频点高度数组 */
uint32_t GreenTime = 0; /* 绿色点下移时间变量 */
uint32_t GreenStopTime[32] = {0}; /* 绿色点顶端停顿时间数据 */

void powerMag ( long nfill );

/* LCD显示红色柱子，RedNewHeight为红色柱子高度列表 */
void music_fft_main ( uint16_t *RedNewHeight, uint16_t *GreenNewHeight ) {
    int BarWidth = 12;
    static uint16_t RedOldHeight[32] = {0};
    static uint16_t GreenOldHeight[32] = {0};

    for ( int i = 1; i < 32; i++ ) {
        LCD_Fill ( ( BarWidth + 3 ) * ( i - 1 ), GreenOldHeight[i], ( BarWidth + 3 ) * ( i - 1 ) + BarWidth, GreenOldHeight[i] + 3, WHITE ); /* 清除以前的绿色方块 */
        LCD_Fill ( ( BarWidth + 3 ) * ( i - 1 ), GreenNewHeight[i], ( BarWidth + 3 ) * ( i - 1 ) + BarWidth, GreenNewHeight[i] + 3, GREEN ); /* 显示当前的绿色方块 */

        if ( RedNewHeight[i] > RedOldHeight[i] ) { /* 如果当前的柱子高度比之前的大，则补齐红色柱子 */
            LCD_Fill ( ( BarWidth + 3 ) * ( i - 1 ), RedOldHeight[i], ( BarWidth + 3 ) * ( i - 1 ) + BarWidth, RedNewHeight[i], RED );
        } else { /* 如果当前的柱子高度小于之前的，则需要将多余的高度用背景色填充 */
            LCD_Fill ( ( BarWidth + 3 ) * ( i - 1 ), RedNewHeight[i], ( BarWidth + 3 ) * ( i - 1 ) + BarWidth, RedOldHeight[i], WHITE );
        }

        RedOldHeight[i] = RedNewHeight[i]; /* 将新数据保存在数组中 */
        GreenOldHeight[i] = GreenNewHeight[i];
    }
}

int main ( void ) {
    DISABLE_JTAG; /* 关闭JTAG功能 */
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    LCD_Init(); /* 显示屏的尺寸是3.5寸 */
    LCD_Clear ( WHITE ); /* 清除屏幕，背景色为白色 */
    POINT_COLOR = RED; /* 前景色为红色 */
    Adc_Init(); /* ADC功能初始化 */
    DMA_Config();
    TIM2_Int_Init ( 25, 71 ); /* 采样频率为40Hz */
    ADC_SoftwareStartConvCmd ( ADC1, DISABLE );

    while ( 1 ) {
        if ( ADC_TimeOutFlag ) {
            GreenTime++;
            RedTime++;
            ADC_TimeOutFlag = 0;

            if ( ADC_DataNum < NPT ) { /* 采样点没有达到所要求的点 */
                ADC_SoftwareStartConvCmd ( ADC1, ENABLE );

                while ( !DMA_GetFlagStatus ( DMA1_FLAG_TC1 ) ); /* 等待DMA1传输完成 */

                DMA_ClearFlag ( DMA1_FLAG_TC1 ); /* 清除DMA1的标志位 */
                ADC_SoftwareStartConvCmd ( ADC1, DISABLE );
                lBUFIN[ADC_DataNum] = ADC_ConvertedValue << 16; /* lBUFIN中每个数据的高16位存储采样数据的实部，低16位存储采样数据的虚部(总是为0) */
                ADC_DataNum++;
            } else { /* 达到采样的点数 */
                TIM_Cmd ( TIM2, DISABLE ); /* 关闭定时器2 */
                ADC_DataNum = 0;
                cr4_fft_64_stm32 ( lBUFOUT, lBUFIN, NPT ); /* 调用STM32的DSP库作FFT变换 */
                powerMag ( NPT ); /* 计算频点幅值 */

                for ( int i = 0; i < NPT / 2; i++ ) {
                    if ( lBUFMAG[i] % 320 > fftHightRedBuf[i] % 320 ) {
                        fftHightRedBuf[i] = lBUFMAG[i] % 320;
                    }

                    if ( fftHightRedBuf[i] >= fftHightGreenBuf[i] ) { /* 刷新绿色点高度 */
                        fftHightGreenBuf[i] = fftHightRedBuf[i];
                        GreenStopTime[i] = GREEN_STOP_TIME; /* 绿点停顿时间 */

                        if ( fftHightRedBuf[i] >= 319 ) {
                            fftHightGreenBuf[i] = 319;
                            fftHightRedBuf[i] = 319;
                        }
                    }
                }

                music_fft_main ( fftHightRedBuf, fftHightGreenBuf );

                if ( ( GreenTime > GREEN_SUB_SPEED ) ) { /* 绿色点下移 */
                    GreenTime = 0;

                    for ( int i = 0; i < NPT / 2; i++ ) {
                        if ( ( fftHightGreenBuf[i] != 0 ) && ( GreenStopTime[i] == 0 ) ) {
                            fftHightGreenBuf[i]--;
                        }
                    }
                }

                if ( RedTime > RED_SUB_SPEED ) { /* 红色柱子下移 */
                    RedTime = 0;

                    for ( int i = 0; i < NPT / 2; i++ ) {
                        if ( ( fftHightRedBuf[i] != 0 ) ) {
                            fftHightRedBuf[i]--;
                        }
                    }
                }

                for ( int i = 0; i < NPT / 2; i++ ) { /* 绿色点停顿时间减一 */
                    if ( GreenStopTime[i] != 0 ) {
                        GreenStopTime[i]--;
                    }
                }

                TIM_Cmd ( TIM2, ENABLE ); /* 使能定时器2 */
            }
        }
    }
}

void powerMag ( long nfill ) { /* 计算FFT后每个频点的幅值 */
    int32_t lX, lY;
    uint32_t i;

    for ( i = 0; i < nfill; i++ ) {
        lX = ( lBUFOUT[i] << 16 ) >> 16; /* sine_cosine -> cos */
        lY = ( lBUFOUT[i] >> 16 ); /* sine_cosine -> sin */
        {
            float X =  64 * ( ( float ) lX ) / 32768;
            float Y = 64 * ( ( float ) lY ) / 32768;
            float Mag = sqrt ( X * X + Y * Y ) / nfill; /* 先平方和，再开方 */
            lBUFMAG[i] = ( long ) ( Mag * 65536 );
        }
    }
}