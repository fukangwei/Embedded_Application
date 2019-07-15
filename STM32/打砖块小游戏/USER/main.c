#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include <math.h>
#include "lcd.h"
#include "my_exit.h"

extern volatile int move_left;
extern volatile int move_right;

void simulate ( void ) {
    int score_num = 0;
    const int r = 20;
    const int l = 320;
    double x_ball = 160, y_ball = 330;
    double vx = 0, vy = 0;
    double x_rect = 160, y_rect = 360;
    const int w_rect = 80;
    const int h_rect = 20;
    const int move_step = 20;
    double left = r;
    double right = l - r;
    double bottom = y_rect - h_rect / 2 - r;
    double top = r;

    LCD_DrawRectangle ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2 );
    LCD_Fill ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2, RED );
    LCD_ShowString ( 0, 450, 50, 50, 16, "score:" );
    LCD_ShowNum ( 50, 450, score_num, 1, 16 );
    vy = sin ( 45 * 3.14 / 180 ) * 5;
    vx = cos ( 45 * 3.14 / 180 ) * 5;

    while ( 1 ) {
        x_ball += vx;
        y_ball -= vy;

        while ( ( x_ball < left ) || ( x_ball > right ) || ( y_ball > bottom ) || ( y_ball < top ) ) {
            if ( x_ball < left ) {
                x_ball = 2 * r - x_ball, vx = -vx;
            }

            if ( x_ball > right ) {
                x_ball = 2 * l - 2 * r - x_ball, vx = -vx;
            }

            if ( ( y_ball > bottom ) && ( ( x_ball > x_rect - w_rect / 2 - r / 1.41 ) && ( x_ball < x_rect + w_rect / 2 + r / 1.41 ) ) ) {
                y_ball = 2 * ( bottom + r ) - 2 * r - y_ball, vy = -vy;
                score_num++;

                if ( ( score_num >= 0 ) && ( score_num <= 9 ) ) {
                    LCD_ShowNum ( 50, 450, score_num, 1, 16 );
                } else if ( ( score_num >= 10 ) && ( score_num <= 99 ) ) {
                    LCD_ShowNum ( 50, 450, score_num, 2, 16 );
                }
            } else if ( ( y_ball > bottom ) && ( ( x_ball < x_rect - w_rect / 2  - r / 1.41 ) || ( x_ball > x_rect + w_rect / 2 + - r / 1.41 ) ) ) {
                LCD_Clear ( GREEN );
                POINT_COLOR = RED;
                LCD_ShowString ( 0, 450, 50, 50, 16, "score:" );
                if ( ( score_num >= 0 ) && ( score_num <= 9 ) ) {
                    LCD_ShowNum ( 50, 450, score_num, 1, 16 );
                } else if ( ( score_num >= 10 ) && ( score_num <= 99 ) ) {
                    LCD_ShowNum ( 50, 450, score_num, 2, 16 );
                }
            }

            if ( y_ball < top ) {
                y_ball = 2 * r - y_ball, vy = -vy;
            }
        }

        Draw_Circle ( x_ball, y_ball, r );
        delay_ms ( 10 );
        LCD_Fill ( x_ball - r, y_ball - r, x_ball + r, y_ball + r, WHITE );

        if ( move_left == 1 ) {
            move_left = 0;

            if ( x_rect > w_rect / 2 ) {
                LCD_Fill ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2, WHITE );
                x_rect -= move_step;
                LCD_Fill ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2, RED );
            }
        }

        if ( move_right == 1 ) {
            move_right = 0;

            if ( x_rect < l - w_rect / 2 ) {
                LCD_Fill ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2, WHITE );
                x_rect += move_step;
                LCD_Fill ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2, RED );
            }
        }
    }

#if 1
    vy = sin ( 45 * 3.14 / 180 ) * 5;
    vx = cos ( 45 * 3.14 / 180 ) * 5;

    while ( 1 ) {
        x_ball += vx;
        y_ball -= vy;

        while ( ( x_ball < left ) || ( x_ball > right ) || ( y_ball > bottom ) || ( y_ball < top ) ) {
            if ( x_ball < left ) {
                x_ball = 2 * r - x_ball, vx = -vx;
            }

            if ( x_ball > right ) {
                x_ball = 2 * l - 2 * r - x_ball, vx = -vx;
            }

            if ( y_ball > bottom ) {
                y_ball = 2 * w - 2 * r - y_ball, vy = -vy;
            }

            if ( y_ball < top ) {
                y_ball = 2 * r - y_ball, vy = -vy;
            }
        }

        Draw_Circle ( x_ball, y_ball, r );
        delay_ms ( 10 );
        LCD_Fill ( x_ball - r, y_ball - r, x_ball + r, y_ball + r, WHITE );
    }
#endif
}

#define DISABLE_JTAG do \
    { \
        RCC->APB2ENR |= 0x00000001; \
        AFIO->MAPR = ( 0x00FFFFFF & AFIO->MAPR ) | 0x04000000; \
    }while(0);

int main ( void ) {
    DISABLE_JTAG;
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    key_init();
    LCD_Init();
    LCD_Clear ( WHITE );
    POINT_COLOR = RED;
    simulate();

    while ( 1 );
}