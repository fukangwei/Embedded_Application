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
    int score_num = 0; /* ���� */
    const int r = 20; /* С��뾶 */
    const int l = 320; /* ��Ļ�ĳ��� */
    double x_ball = 160, y_ball = 330; /* С���ʵʱλ�� */
    double vx = 0, vy = 0; /* x��y�����ϵ��ٶȷ��� */
    /*----------------------------------------------------*/
    double x_rect = 160, y_rect = 360; /* ���þ��������� */
    const int w_rect = 80; /* ���þ����Ŀ�� */
    const int h_rect = 20; /* ���þ����ĸ߶� */
    const int move_step = 20; /* ���þ�����ƶ��Ĳ��� */
    /*-------------------------------------------------------*/
    double left = r; /* �߽���� */
    double right = l - r; /* �߽��Ҳ� */
    double bottom = y_rect - h_rect / 2 - r; /* �߽�ײ� */
    double top = r; /* �߽��ϲ� */
    /*----------------------------------------------------*/
    LCD_DrawRectangle ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2 ); /* ��������� */
    LCD_Fill ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2, RED );
    /*----------------------------------------------------*/
    LCD_ShowString ( 0, 450, 50, 50, 16, "score:" );
    LCD_ShowNum ( 50, 450, score_num, 1, 16 );
    vy = sin ( 45 * 3.14 / 180 ) * 5; /* x�������ٶȷ��� */
    vx = cos ( 45 * 3.14 / 180 ) * 5; /* y�������ٶȷ��� */

    while ( 1 ) {
        x_ball += vx; /* ����x����ķ��� */
        y_ball -= vy; /* ����y����ķ��� */

        while ( ( x_ball < left ) || ( x_ball > right ) || ( y_ball > bottom ) || ( y_ball < top ) ) {
            if ( x_ball < left ) { /* ������� */
                x_ball = 2 * r - x_ball, vx = -vx;
            }

            if ( x_ball > right ) { /* �����Ҳ� */
                x_ball = 2 * l - 2 * r - x_ball, vx = -vx;
            }

            if ( ( y_ball > bottom ) && ( ( x_ball > x_rect - w_rect / 2 - r / 1.41 ) && ( x_ball < x_rect + w_rect / 2 + r / 1.41 ) ) ) { /* С�򴥵������ */
                y_ball = 2 * ( bottom + r ) - 2 * r - y_ball, vy = -vy;
                score_num++;

                if ( ( score_num >= 0 ) && ( score_num <= 9 ) ) {
                    LCD_ShowNum ( 50, 450, score_num, 1, 16 );
                } else if ( ( score_num >= 10 ) && ( score_num <= 99 ) ) {
                    LCD_ShowNum ( 50, 450, score_num, 2, 16 );
                }
            } else if ( ( y_ball > bottom ) && ( ( x_ball < x_rect - w_rect / 2  - r / 1.41 ) || ( x_ball > x_rect + w_rect / 2 + - r / 1.41 ) ) ) { /* С��δ��������� */
                LCD_Clear ( GREEN );
                POINT_COLOR = RED;
                LCD_ShowString ( 0, 450, 50, 50, 16, "score:" );
                if ( ( score_num >= 0 ) && ( score_num <= 9 ) ) {
                    LCD_ShowNum ( 50, 450, score_num, 1, 16 );
                } else if ( ( score_num >= 10 ) && ( score_num <= 99 ) ) {
                    LCD_ShowNum ( 50, 450, score_num, 2, 16 );
                }
            }

            if ( y_ball < top ) { /* �������� */
                y_ball = 2 * r - y_ball, vy = -vy;
            }
        }

        Draw_Circle ( x_ball, y_ball, r );
        delay_ms ( 10 );
        LCD_Fill ( x_ball - r, y_ball - r, x_ball + r, y_ball + r, WHITE );

        /*-------------------------------------------------------------------------------------------------------------*/
        if ( move_left == 1 ) { /* ����������ƶ� */
            move_left = 0; /* ���ƶ���־���и�λ */

            if ( x_rect > w_rect / 2 ) { /* ���δ�ƶ������Ե */
                LCD_Fill ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2, WHITE );
                x_rect -= move_step;
                LCD_Fill ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2, RED );
            }
        }

        if ( move_right == 1 ) { /* ����������ƶ� */
            move_right = 0; /* ���ƶ���־���и�λ */

            if ( x_rect < l - w_rect / 2 ) { /* ���δ�ƶ����ұ�Ե */
                LCD_Fill ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2, WHITE );
                x_rect += move_step;
                LCD_Fill ( x_rect - w_rect / 2,  y_rect - h_rect / 2, x_rect + w_rect / 2, y_rect + h_rect / 2, RED );
            }
        }
    }

#if 0
    vy = sin ( 45 * 3.14 / 180 ) * 5; /* x�������ٶȷ��� */
    vx = cos ( 45 * 3.14 / 180 ) * 5; /* y�������ٶȷ��� */

    while ( 1 ) {
        x_ball += vx; /* ����x����ķ��� */
        y_ball -= vy; /* ����y����ķ��� */

        while ( ( x_ball < left ) || ( x_ball > right ) || ( y_ball > bottom ) || ( y_ball < top ) ) {
            if ( x_ball < left ) { /* ������� */
                x_ball = 2 * r - x_ball, vx = -vx;
            }

            if ( x_ball > right ) { /* �����Ҳ� */
                x_ball = 2 * l - 2 * r - x_ball, vx = -vx;
            }

            if ( y_ball > bottom ) { /* �����ײ� */
                y_ball = 2 * w - 2 * r - y_ball, vy = -vy;
            }

            if ( y_ball < top ) { /* �������� */
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
        RCC->APB2ENR |= 0x00000001; /* ����afioʱ�� */ \
        AFIO->MAPR = ( 0x00FFFFFF & AFIO->MAPR ) | 0x04000000; /* �ر�JTAG */ \
    }while(0);

int main ( void ) {
    DISABLE_JTAG; /* �ر�JTAG���� */
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

