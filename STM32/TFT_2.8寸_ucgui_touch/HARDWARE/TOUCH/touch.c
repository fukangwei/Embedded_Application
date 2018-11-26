#include "touch.h"
#include "ILI93xx.h"
#include "delay.h"
#include "stdlib.h"
#include "math.h"
#include "24cxx.h"

Pen_Holder Pen_Point;

u8 CMD_RDX = 0XD0;
u8 CMD_RDY = 0X90;

void ADS_Write_Byte ( u8 num ) {
    u8 count = 0;

    for ( count = 0; count < 8; count++ ) {
        if ( num & 0x80 ) {
            TDIN = 1;
        } else {
            TDIN = 0;
        }

        num <<= 1;
        TCLK = 0;
        TCLK = 1;
    }
}

u16 ADS_Read_AD ( u8 CMD ) {
    u8 count = 0;
    u16 Num = 0;
    TCLK = 0;
    TCS = 0;
    ADS_Write_Byte ( CMD );
    delay_us ( 6 );
    TCLK = 1;
    TCLK = 0;

    for ( count = 0; count < 15; count++ ) {
        Num <<= 1;
        TCLK = 1;
        TCLK = 0;

        if ( DOUT ) {
            Num++;
        }
    }

    Num >>= 3;
    TCS = 1;
    return ( Num );
}

#define READ_TIMES 15
#define LOST_VAL 5

u16 ADS_Read_XY ( u8 xy ) {
    u16 i, j;
    u16 buf[READ_TIMES];
    u16 sum = 0;
    u16 temp;

    for ( i = 0; i < READ_TIMES; i++ ) {
        buf[i] = ADS_Read_AD ( xy );
    }

    for ( i = 0; i < READ_TIMES - 1; i++ ) {
        for ( j = i + 1; j < READ_TIMES; j++ ) {
            if ( buf[i] > buf[j] ) {
                temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }

    sum = 0;

    for ( i = LOST_VAL; i < READ_TIMES - LOST_VAL; i++ ) {
        sum += buf[i];
    }

    temp = sum / ( READ_TIMES - 2 * LOST_VAL );
    return temp;
}

u8 Read_ADS ( u16 *x, u16 *y ) {
    u16 xtemp, ytemp;
    xtemp = ADS_Read_XY ( CMD_RDX );
    ytemp = ADS_Read_XY ( CMD_RDY );

    if ( xtemp < 100 || ytemp < 100 ) {
        return 0;
    }

    *x = xtemp;
    *y = ytemp;
    return 1;
}

#define ERR_RANGE 50

u8 Read_ADS2 ( u16 *x, u16 *y ) {
    u16 x1, y1;
    u16 x2, y2;
    u8 flag;
    flag = Read_ADS ( &x1, &y1 );

    if ( flag == 0 ) {
        return ( 0 );
    }

    flag = Read_ADS ( &x2, &y2 );

    if ( flag == 0 ) {
        return ( 0 );
    }

    if ( ( ( x2 <= x1 && x1 < x2 + ERR_RANGE ) || ( x1 <= x2 && x2 < x1 + ERR_RANGE ) )
         && ( ( y2 <= y1 && y1 < y2 + ERR_RANGE ) || ( y1 <= y2 && y2 < y1 + ERR_RANGE ) ) ) {
        *x = ( x1 + x2 ) / 2;
        *y = ( y1 + y2 ) / 2;
        return 1;
    } else {
        return 0;
    }
}

u8 Read_TP_Once ( void ) {
    u8 t = 0;
    Pen_Int_Set ( 0 );
    Pen_Point.Key_Sta = Key_Up;
    Read_ADS2 ( &Pen_Point.X, &Pen_Point.Y );

    while ( PEN == 0 && t <= 250 ) {
        t++;
        delay_ms ( 10 );
    };

    Pen_Int_Set ( 1 );

    if ( t >= 250 ) {
        return 0;
    } else {
        return 1;
    }
}

void Drow_Touch_Point ( u8 x, u16 y ) {
    LCD_DrawLine ( x - 12, y, x + 13, y );
    LCD_DrawLine ( x, y - 12, x, y + 13 );
    LCD_DrawPoint ( x + 1, y + 1 );
    LCD_DrawPoint ( x - 1, y + 1 );
    LCD_DrawPoint ( x + 1, y - 1 );
    LCD_DrawPoint ( x - 1, y - 1 );
    Draw_Circle ( x, y, 6 );
}

void Draw_Big_Point ( u8 x, u16 y ) {
    LCD_DrawPoint ( x, y );
    LCD_DrawPoint ( x + 1, y );
    LCD_DrawPoint ( x, y + 1 );
    LCD_DrawPoint ( x + 1, y + 1 );
}

void Convert_Pos ( void ) {
    if ( Read_ADS2 ( &Pen_Point.X, &Pen_Point.Y ) ) {
        Pen_Point.X0 = Pen_Point.xfac * Pen_Point.X + Pen_Point.xoff;
        Pen_Point.Y0 = Pen_Point.yfac * Pen_Point.Y + Pen_Point.yoff;
    }
}

void EXTI1_IRQHandler ( void ) {
    Pen_Point.Key_Sta = Key_Down;
    EXTI->PR = 1 << 1;
}

void Pen_Int_Set ( u8 en ) {
    if ( en ) {
        EXTI->IMR |= 1 << 1;
    } else {
        EXTI->IMR &= ~ ( 1 << 1 );
    }
}

#ifdef ADJ_SAVE_ENABLE
#define SAVE_ADDR_BASE 40
void Save_Adjdata ( void ) {
    s32 temp;
    temp = Pen_Point.xfac * 100000000;
    AT24CXX_WriteLenByte ( SAVE_ADDR_BASE, temp, 4 );
    temp = Pen_Point.yfac * 100000000;
    AT24CXX_WriteLenByte ( SAVE_ADDR_BASE + 4, temp, 4 );
    AT24CXX_WriteLenByte ( SAVE_ADDR_BASE + 8, Pen_Point.xoff, 2 );
    AT24CXX_WriteLenByte ( SAVE_ADDR_BASE + 10, Pen_Point.yoff, 2 );
    AT24CXX_WriteOneByte ( SAVE_ADDR_BASE + 12, Pen_Point.touchtype );
    temp = 0X0A;
    AT24CXX_WriteOneByte ( SAVE_ADDR_BASE + 13, temp );
}

u8 Get_Adjdata ( void ) {
    s32 tempfac;
    tempfac = AT24CXX_ReadOneByte ( SAVE_ADDR_BASE + 13 );

    if ( tempfac == 0X0A ) {
        tempfac = AT24CXX_ReadLenByte ( SAVE_ADDR_BASE, 4 );
        Pen_Point.xfac = ( float ) tempfac / 100000000;
        tempfac = AT24CXX_ReadLenByte ( SAVE_ADDR_BASE + 4, 4 );
        Pen_Point.yfac = ( float ) tempfac / 100000000;
        Pen_Point.xoff = AT24CXX_ReadLenByte ( SAVE_ADDR_BASE + 8, 2 );
        Pen_Point.yoff = AT24CXX_ReadLenByte ( SAVE_ADDR_BASE + 10, 2 );
        Pen_Point.touchtype = AT24CXX_ReadOneByte ( SAVE_ADDR_BASE + 12 );

        if ( Pen_Point.touchtype ) {
            CMD_RDX = 0X90;
            CMD_RDY = 0XD0;
        } else {
            CMD_RDX = 0XD0;
            CMD_RDY = 0X90;
        }

        return 1;
    }

    return 0;
}
#endif

void ADJ_INFO_SHOW ( u8 *str ) {
    LCD_ShowString ( 40, 40, "x1:       y1:       " );
    LCD_ShowString ( 40, 60, "x2:       y2:       " );
    LCD_ShowString ( 40, 80, "x3:       y3:       " );
    LCD_ShowString ( 40, 100, "x4:       y4:       " );
    LCD_ShowString ( 40, 100, "x4:       y4:       " );
    LCD_ShowString ( 40, 120, str );
}

void Touch_Adjust ( void ) {
    signed short pos_temp[4][2];
    u8  cnt = 0;
    u16 d1, d2;
    u32 tem1, tem2;
    float fac;
    cnt = 0;
    POINT_COLOR = BLUE;
    BACK_COLOR = WHITE;
    LCD_Clear ( WHITE );
    POINT_COLOR = RED;
    LCD_Clear ( WHITE );
    Drow_Touch_Point ( 20, 20 );
    Pen_Point.Key_Sta = Key_Up;
    Pen_Point.xfac = 0;

    while ( 1 ) {
        if ( Pen_Point.Key_Sta == Key_Down ) {
            if ( Read_TP_Once() ) {
                pos_temp[cnt][0] = Pen_Point.X;
                pos_temp[cnt][1] = Pen_Point.Y;
                cnt++;
            }

            switch ( cnt ) {
                case 1:
                    LCD_Clear ( WHITE );
                    Drow_Touch_Point ( 220, 20 );
                    break;

                case 2:
                    LCD_Clear ( WHITE );
                    Drow_Touch_Point ( 20, 300 );
                    break;

                case 3:
                    LCD_Clear ( WHITE );
                    Drow_Touch_Point ( 220, 300 );
                    break;

                case 4:
                    tem1 = abs ( pos_temp[0][0] - pos_temp[1][0] );
                    tem2 = abs ( pos_temp[0][1] - pos_temp[1][1] );
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d1 = sqrt ( tem1 + tem2 );
                    tem1 = abs ( pos_temp[2][0] - pos_temp[3][0] );
                    tem2 = abs ( pos_temp[2][1] - pos_temp[3][1] );
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d2 = sqrt ( tem1 + tem2 );
                    fac = ( float ) d1 / d2;

                    if ( fac < 0.95 || fac > 1.05 || d1 == 0 || d2 == 0 ) {
                        cnt = 0;
                        LCD_Clear ( WHITE );
                        Drow_Touch_Point ( 20, 20 );
                        ADJ_INFO_SHOW ( "ver fac is:" );
                        LCD_ShowNum ( 40 + 24, 40, pos_temp[0][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 40, pos_temp[0][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 60, pos_temp[1][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 60, pos_temp[1][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 80, pos_temp[2][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 80, pos_temp[2][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 100, pos_temp[3][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 100, pos_temp[3][1], 4, 16 );
                        LCD_ShowNum ( 40, 140, fac * 100, 3, 16 );
                        continue;
                    }

                    tem1 = abs ( pos_temp[0][0] - pos_temp[2][0] );
                    tem2 = abs ( pos_temp[0][1] - pos_temp[2][1] );
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d1 = sqrt ( tem1 + tem2 );
                    tem1 = abs ( pos_temp[1][0] - pos_temp[3][0] );
                    tem2 = abs ( pos_temp[1][1] - pos_temp[3][1] );
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d2 = sqrt ( tem1 + tem2 );
                    fac = ( float ) d1 / d2;

                    if ( fac < 0.95 || fac > 1.05 ) {
                        cnt = 0;
                        LCD_Clear ( WHITE );
                        Drow_Touch_Point ( 20, 20 );
                        ADJ_INFO_SHOW ( "hor fac is:" );
                        LCD_ShowNum ( 40 + 24, 40, pos_temp[0][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 40, pos_temp[0][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 60, pos_temp[1][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 60, pos_temp[1][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 80, pos_temp[2][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 80, pos_temp[2][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 100, pos_temp[3][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 100, pos_temp[3][1], 4, 16 );
                        LCD_ShowNum ( 40, 140, fac * 100, 3, 16 );
                        continue;
                    }

                    tem1 = abs ( pos_temp[1][0] - pos_temp[2][0] );
                    tem2 = abs ( pos_temp[1][1] - pos_temp[2][1] );
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d1 = sqrt ( tem1 + tem2 );
                    tem1 = abs ( pos_temp[0][0] - pos_temp[3][0] );
                    tem2 = abs ( pos_temp[0][1] - pos_temp[3][1] );
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d2 = sqrt ( tem1 + tem2 );
                    fac = ( float ) d1 / d2;

                    if ( fac < 0.95 || fac > 1.05 ) {
                        cnt = 0;
                        LCD_Clear ( WHITE );
                        Drow_Touch_Point ( 20, 20 );
                        ADJ_INFO_SHOW ( "dia fac is:" );
                        LCD_ShowNum ( 40 + 24, 40, pos_temp[0][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 40, pos_temp[0][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 60, pos_temp[1][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 60, pos_temp[1][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 80, pos_temp[2][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 80, pos_temp[2][1], 4, 16 );
                        LCD_ShowNum ( 40 + 24, 100, pos_temp[3][0], 4, 16 );
                        LCD_ShowNum ( 40 + 24 + 80, 100, pos_temp[3][1], 4, 16 );
                        LCD_ShowNum ( 40, 140, fac * 100, 3, 16 );
                        continue;
                    }

                    Pen_Point.xfac = ( float ) 200 / ( pos_temp[1][0] - pos_temp[0][0] );
                    Pen_Point.xoff = ( 240 - Pen_Point.xfac * ( pos_temp[1][0] + pos_temp[0][0] ) ) / 2;
                    Pen_Point.yfac = ( float ) 280 / ( pos_temp[2][1] - pos_temp[0][1] );
                    Pen_Point.yoff = ( 320 - Pen_Point.yfac * ( pos_temp[2][1] + pos_temp[0][1] ) ) / 2;

                    if ( abs ( Pen_Point.xfac ) > 2 || abs ( Pen_Point.yfac ) > 2 ) {
                        cnt = 0;
                        LCD_Clear ( WHITE );
                        Drow_Touch_Point ( 20, 20 );
                        LCD_ShowString ( 35, 110, "TP Need readjust!" );
                        Pen_Point.touchtype = !Pen_Point.touchtype;

                        if ( Pen_Point.touchtype ) {
                            CMD_RDX = 0X90;
                            CMD_RDY = 0XD0;
                        } else {
                            CMD_RDX = 0XD0;
                            CMD_RDY = 0X90;
                        }

                        delay_ms ( 500 );
                        continue;
                    }

                    POINT_COLOR = BLUE;
                    LCD_Clear ( WHITE );
                    LCD_ShowString ( 35, 110, "Touch Screen Adjust OK!" );
                    delay_ms ( 500 );
                    LCD_Clear ( WHITE );
                    return;
            }
        }
    }
}

void Touch_Init ( void ) {
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOC  | RCC_APB2Periph_AFIO, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_0 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init ( GPIOC, &GPIO_InitStructure );
    Read_ADS ( &Pen_Point.X, &Pen_Point.Y );
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init ( &NVIC_InitStructure );
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_AFIO, ENABLE );
    GPIO_EXTILineConfig ( GPIO_PortSourceGPIOC, GPIO_PinSource1 );
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init ( &EXTI_InitStructure );
#ifdef ADJ_SAVE_ENABLE
    AT24CXX_Init();

    if ( Get_Adjdata() ) {
        return;
    } else {
        LCD_Clear ( WHITE );
        Touch_Adjust();
        Save_Adjdata();
    }

    Get_Adjdata();
#else
    LCD_Clear ( WHITE );
    Touch_Adjust();
#endif
}