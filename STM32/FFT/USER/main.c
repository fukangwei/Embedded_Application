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

#define NPT 64 /* FFT�������� */
#define RED_SUB_SPEED  60 /* ��ɫƵ�����������ٶȣ�ֵԽ���ٶ�Խ�� */
#define GREEN_SUB_SPEED 100 /* ��ɫ�������ٶȣ�ֵԽ���ٶ�Խ�� */
#define GREEN_STOP_TIME 10 /* ��ɫ�㶥��ͣ��ʱ�䣬ֵԽ��ʱ��Խ�� */

#define DISABLE_JTAG do \
    { \
        RCC->APB2ENR |= 0x00000001; /* ����afioʱ�� */ \
        AFIO->MAPR = ( 0x00FFFFFF & AFIO->MAPR ) | 0x04000000; /* �ر�JTAG */ \
    }while(0);

extern u32 ADC_ConvertedValue;
extern volatile uint8_t ADC_TimeOutFlag;

uint32_t ADC_DataNum = 0; /* ADC��ǰ�������� */
unsigned long lBUFOUT[NPT] = {0};
unsigned long lBUFIN[NPT] = {0};
unsigned long lBUFMAG[NPT + NPT / 2]; /* �洢��ģ������� */
uint16_t fftHightRedBuf[NPT / 2] = {0}; /* ��ɫƵ���߶����� */
uint32_t RedTime = 0; /* ��ɫ������ʱ����� */
uint16_t fftHightGreenBuf[NPT / 2] = {0}; /* ��ɫƵ��߶����� */
uint32_t GreenTime = 0; /* ��ɫ������ʱ����� */
uint32_t GreenStopTime[32] = {0}; /* ��ɫ�㶥��ͣ��ʱ������ */

void powerMag ( long nfill );

/* LCD��ʾ��ɫ���ӣ�RedNewHeightΪ��ɫ���Ӹ߶��б� */
void music_fft_main ( uint16_t *RedNewHeight, uint16_t *GreenNewHeight ) {
    int BarWidth = 12;
    static uint16_t RedOldHeight[32] = {0};
    static uint16_t GreenOldHeight[32] = {0};

    for ( int i = 1; i < 32; i++ ) {
        LCD_Fill ( ( BarWidth + 3 ) * ( i - 1 ), GreenOldHeight[i], ( BarWidth + 3 ) * ( i - 1 ) + BarWidth, GreenOldHeight[i] + 3, WHITE ); /* �����ǰ����ɫ���� */
        LCD_Fill ( ( BarWidth + 3 ) * ( i - 1 ), GreenNewHeight[i], ( BarWidth + 3 ) * ( i - 1 ) + BarWidth, GreenNewHeight[i] + 3, GREEN ); /* ��ʾ��ǰ����ɫ���� */

        if ( RedNewHeight[i] > RedOldHeight[i] ) { /* �����ǰ�����Ӹ߶ȱ�֮ǰ�Ĵ������ɫ���� */
            LCD_Fill ( ( BarWidth + 3 ) * ( i - 1 ), RedOldHeight[i], ( BarWidth + 3 ) * ( i - 1 ) + BarWidth, RedNewHeight[i], RED );
        } else { /* �����ǰ�����Ӹ߶�С��֮ǰ�ģ�����Ҫ������ĸ߶��ñ���ɫ��� */
            LCD_Fill ( ( BarWidth + 3 ) * ( i - 1 ), RedNewHeight[i], ( BarWidth + 3 ) * ( i - 1 ) + BarWidth, RedOldHeight[i], WHITE );
        }

        RedOldHeight[i] = RedNewHeight[i]; /* �������ݱ����������� */
        GreenOldHeight[i] = GreenNewHeight[i];
    }
}

int main ( void ) {
    DISABLE_JTAG; /* �ر�JTAG���� */
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    LCD_Init(); /* ��ʾ���ĳߴ���3.5�� */
    LCD_Clear ( WHITE ); /* �����Ļ������ɫΪ��ɫ */
    POINT_COLOR = RED; /* ǰ��ɫΪ��ɫ */
    Adc_Init(); /* ADC���ܳ�ʼ�� */
    DMA_Config();
    TIM2_Int_Init ( 25, 71 ); /* ����Ƶ��Ϊ40Hz */
    ADC_SoftwareStartConvCmd ( ADC1, DISABLE );

    while ( 1 ) {
        if ( ADC_TimeOutFlag ) {
            GreenTime++;
            RedTime++;
            ADC_TimeOutFlag = 0;

            if ( ADC_DataNum < NPT ) { /* ������û�дﵽ��Ҫ��ĵ� */
                ADC_SoftwareStartConvCmd ( ADC1, ENABLE );

                while ( !DMA_GetFlagStatus ( DMA1_FLAG_TC1 ) ); /* �ȴ�DMA1������� */

                DMA_ClearFlag ( DMA1_FLAG_TC1 ); /* ���DMA1�ı�־λ */
                ADC_SoftwareStartConvCmd ( ADC1, DISABLE );
                lBUFIN[ADC_DataNum] = ADC_ConvertedValue << 16; /* lBUFIN��ÿ�����ݵĸ�16λ�洢�������ݵ�ʵ������16λ�洢�������ݵ��鲿(����Ϊ0) */
                ADC_DataNum++;
            } else { /* �ﵽ�����ĵ��� */
                TIM_Cmd ( TIM2, DISABLE ); /* �رն�ʱ��2 */
                ADC_DataNum = 0;
                cr4_fft_64_stm32 ( lBUFOUT, lBUFIN, NPT ); /* ����STM32��DSP����FFT�任 */
                powerMag ( NPT ); /* ����Ƶ���ֵ */

                for ( int i = 0; i < NPT / 2; i++ ) {
                    if ( lBUFMAG[i] % 320 > fftHightRedBuf[i] % 320 ) {
                        fftHightRedBuf[i] = lBUFMAG[i] % 320;
                    }

                    if ( fftHightRedBuf[i] >= fftHightGreenBuf[i] ) { /* ˢ����ɫ��߶� */
                        fftHightGreenBuf[i] = fftHightRedBuf[i];
                        GreenStopTime[i] = GREEN_STOP_TIME; /* �̵�ͣ��ʱ�� */

                        if ( fftHightRedBuf[i] >= 319 ) {
                            fftHightGreenBuf[i] = 319;
                            fftHightRedBuf[i] = 319;
                        }
                    }
                }

                music_fft_main ( fftHightRedBuf, fftHightGreenBuf );

                if ( ( GreenTime > GREEN_SUB_SPEED ) ) { /* ��ɫ������ */
                    GreenTime = 0;

                    for ( int i = 0; i < NPT / 2; i++ ) {
                        if ( ( fftHightGreenBuf[i] != 0 ) && ( GreenStopTime[i] == 0 ) ) {
                            fftHightGreenBuf[i]--;
                        }
                    }
                }

                if ( RedTime > RED_SUB_SPEED ) { /* ��ɫ�������� */
                    RedTime = 0;

                    for ( int i = 0; i < NPT / 2; i++ ) {
                        if ( ( fftHightRedBuf[i] != 0 ) ) {
                            fftHightRedBuf[i]--;
                        }
                    }
                }

                for ( int i = 0; i < NPT / 2; i++ ) { /* ��ɫ��ͣ��ʱ���һ */
                    if ( GreenStopTime[i] != 0 ) {
                        GreenStopTime[i]--;
                    }
                }

                TIM_Cmd ( TIM2, ENABLE ); /* ʹ�ܶ�ʱ��2 */
            }
        }
    }
}

void powerMag ( long nfill ) { /* ����FFT��ÿ��Ƶ��ķ�ֵ */
    int32_t lX, lY;
    uint32_t i;

    for ( i = 0; i < nfill; i++ ) {
        lX = ( lBUFOUT[i] << 16 ) >> 16; /* sine_cosine -> cos */
        lY = ( lBUFOUT[i] >> 16 ); /* sine_cosine -> sin */
        {
            float X =  64 * ( ( float ) lX ) / 32768;
            float Y = 64 * ( ( float ) lY ) / 32768;
            float Mag = sqrt ( X * X + Y * Y ) / nfill; /* ��ƽ���ͣ��ٿ��� */
            lBUFMAG[i] = ( long ) ( Mag * 65536 );
        }
    }
}