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
    int score_num = 0; /* 分数 */
    const int r = 20; /* 小球半径 */
    const int l = 320; /* 屏幕的长度 */
    double x_ball = 160, y_ball = 330; /* 小球的实时位置 */
    double vx = 0, vy = 0; /* x、y方向上的速度分量 */
    /*----------------------------------------------------*/
    double x_rect = 160, y_rect = 360; /* 设置矩阵块的中心 */
    const int w_rect = 80; /* 设置矩阵块的宽度 */
    const int h_rect = 20; /* 设置矩阵块的高度 */
    const int move_step = 20; /* 设置矩阵块移动的步长 */
    /*-------------------------------------------------------*/
    double left = r; /* 边界左侧 */
    double right = l - r; /* 边界右侧 */
    double bottom = y_rect - h_rect / 2 - r; /* 边界底部 */
    double top = r; /* 边界上部 */
    /*----------------------------------------------------*/
    LCD_DrawRectangle ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2 ); /* 画出矩阵块 */
    LCD_Fill ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2, RED );
    /*----------------------------------------------------*/
    LCD_ShowString ( 0, 450, 50, 50, 16, "score:" );
    LCD_ShowNum ( 50, 450, score_num, 1, 16 );
    vy = sin ( 45 * 3.14 / 180 ) * 5; /* x方向上速度分量 */
    vx = cos ( 45 * 3.14 / 180 ) * 5; /* y方向上速度分量 */

    while ( 1 ) {
        x_ball += vx; /* 增加x方向的分量 */
        y_ball -= vy; /* 增加y方向的分量 */

        while ( ( x_ball < left ) || ( x_ball > right ) || ( y_ball > bottom ) || ( y_ball < top ) ) {
            if ( x_ball < left ) { /* 超过左侧 */
                x_ball = 2 * r - x_ball, vx = -vx;
            }

            if ( x_ball > right ) { /* 超过右侧 */
                x_ball = 2 * l - 2 * r - x_ball, vx = -vx;
            }

            if ( ( y_ball > bottom ) && ( ( x_ball > x_rect - w_rect / 2 - r / 1.41 ) && ( x_ball < x_rect + w_rect / 2 + r / 1.41 ) ) ) { /* 小球触到矩阵块 */
                y_ball = 2 * ( bottom + r ) - 2 * r - y_ball, vy = -vy;
                score_num++;

                if ( ( score_num >= 0 ) && ( score_num <= 9 ) ) {
                    LCD_ShowNum ( 50, 450, score_num, 1, 16 );
                } else if ( ( score_num >= 10 ) && ( score_num <= 99 ) ) {
                    LCD_ShowNum ( 50, 450, score_num, 2, 16 );
                }
            } else if ( ( y_ball > bottom ) && ( ( x_ball < x_rect - w_rect / 2  - r / 1.41 ) || ( x_ball > x_rect + w_rect / 2 + - r / 1.41 ) ) ) { /* 小球未触到矩阵块 */
                LCD_Clear ( GREEN );
                POINT_COLOR = RED;
                LCD_ShowString ( 0, 450, 50, 50, 16, "score:" );
                if ( ( score_num >= 0 ) && ( score_num <= 9 ) ) {
                    LCD_ShowNum ( 50, 450, score_num, 1, 16 );
                } else if ( ( score_num >= 10 ) && ( score_num <= 99 ) ) {
                    LCD_ShowNum ( 50, 450, score_num, 2, 16 );
                }
            }

            if ( y_ball < top ) { /* 超过顶部 */
                y_ball = 2 * r - y_ball, vy = -vy;
            }
        }

        Draw_Circle ( x_ball, y_ball, r );
        delay_ms ( 10 );
        LCD_Fill ( x_ball - r, y_ball - r, x_ball + r, y_ball + r, WHITE );

        /*-------------------------------------------------------------------------------------------------------------*/
        if ( move_left == 1 ) { /* 矩阵块向左移动 */
            move_left = 0; /* 对移动标志进行复位 */

            if ( x_rect > w_rect / 2 ) { /* 如果未移动到左边缘 */
                LCD_Fill ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2, WHITE );
                x_rect -= move_step;
                LCD_Fill ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2, RED );
            }
        }

        if ( move_right == 1 ) { /* 矩阵块向右移动 */
            move_right = 0; /* 对移动标志进行复位 */

            if ( x_rect < l - w_rect / 2 ) { /* 如果未移动到右边缘 */
                LCD_Fill ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2, WHITE );
                x_rect += move_step;
                LCD_Fill ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2, RED );
            }
        }
    }

#if 0
    vy = sin ( 45 * 3.14 / 180 ) * 5; /* x方向上速度分量 */
    vx = cos ( 45 * 3.14 / 180 ) * 5; /* y方向上速度分量 */

    while ( 1 ) {
        x_ball += vx; /* 增加x方向的分量 */
        y_ball -= vy; /* 增加y方向的分量 */

        while ( ( x_ball < left ) || ( x_ball > right ) || ( y_ball > bottom ) || ( y_ball < top ) ) {
            if ( x_ball < left ) { /* 超过左侧 */
                x_ball = 2 * r - x_ball, vx = -vx;
            }

            if ( x_ball > right ) { /* 超过右侧 */
                x_ball = 2 * l - 2 * r - x_ball, vx = -vx;
            }

            if ( y_ball > bottom ) { /* 超过底部 */
                y_ball = 2 * w - 2 * r - y_ball, vy = -vy;
            }

            if ( y_ball < top ) { /* 超过顶部 */
                y_ball = 2 * r - y_ball, vy = -vy;
            }
        }

        Draw_Circle ( x_ball, y_ball, r );
        delay_ms ( 10 );
        LCD_Fill ( x_ball - r, y_ball - r, x_ball + r, y_ball + r, WHITE );
    }

#endif
}


/*-------------------------------------------------------*/

#define DISABLE_JTAG do \
    { \
        RCC->APB2ENR |= 0x00000001; /* 开启afio时钟 */ \
        AFIO->MAPR = ( 0x00FFFFFF & AFIO->MAPR ) | 0x04000000; /* 关闭JTAG */ \
    }while(0);

int main ( void ) {
    DISABLE_JTAG; /* 关闭JTAG功能 */
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

