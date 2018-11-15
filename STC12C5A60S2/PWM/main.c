#include "STC12C5A.h"

void main ( void ) {
    CMOD = 0x00; /* Setup PCA timer, Fosc/12(空闲模式下PCA计数器继续工作) */
    CL = 0x00;
    CH = 0x00;
    CCAP0L = 0x40; /* Set the initial value same as CCAP0H */
    CCAP0H = 0x40; /* 75% Duty Cycle */
    CCAPM0 = 0x42; /* 0100_0010 Setup PCA module 0 in PWM mode */
    CR = 1; /* Start PCA Timer */

    while ( 1 );
}