#include "STC12C5A.H"

sbit LIGHT   = P1 ^ 5; /* 背光线                       */
sbit SCLK    = P1 ^ 4; /* 串行时钟线                   */
sbit SDIN    = P1 ^ 3; /* 串行数据线                   */
sbit LCD_DC  = P1 ^ 2; /* 模式选择：1为写数据，0为写指令 */
sbit LCD_CE  = P1 ^ 1; /* 芯片使能                     */
sbit LCD_RST = P1 ^ 0; /* 复位，低电平有效              */

void LCD_init ( void );
void LCD_clear ( void );
void LCD_move_chinese_string ( unsigned char X, unsigned char Y, unsigned char T );
void LCD_write_english_string ( unsigned char X, unsigned char Y, char* s );
void LCD_write_chinese_string ( unsigned char X, unsigned char Y,
                                unsigned char ch_with, unsigned char num,
                                unsigned char line, unsigned char row );
void chinese_string ( unsigned char X, unsigned char Y, unsigned char T );
void LCD_write_char ( unsigned char c );
void LCD_draw_bmp_pixel ( unsigned char X, unsigned char Y, unsigned char* map,
                          unsigned char Pix_x, unsigned char Pix_y );
void LCD_write_byte ( unsigned char dat, unsigned char dc );
void LCD_set_XY ( unsigned char X, unsigned char Y );
void delay_1us ( void );