#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "mmc_sd.h"
#include "ff.h"
#include "photo.h"

FATFS fs;
FIL fsrc;
UINT bw;

unsigned char RecPhoto ( unsigned char tmp );

int main ( void ) {
    unsigned char buffer;
    int i;
    NVIC_Configuration();
    delay_init();
    uart_init ( 9600 );
    printf ( "SD Card TEST\r\n" );

    while ( SD_Initialize() != 0 ) {
        printf ( "SD Card Failed!\r\n" );
        delay_ms ( 500 );
        printf ( "Please Check!\r\n", );
        delay_ms ( 500 );
    }

    printf ( "SD Card Checked OK\r\n" );
    f_mount ( 0, &fs );
    f_open ( &fsrc, "20150520121102.jpg", FA_OPEN_ALWAYS | FA_WRITE );

    for ( i = 0; ; i++ ) {
        buffer = photo[i];
        f_write ( &fsrc, &buffer, sizeof ( buffer ), &bw );

        if ( RecPhoto ( buffer ) == 0 ) {
            break;
        }
    }

    f_close ( &fsrc );
    f_mount ( 0, NULL );
    printf ( "Write OK\r\n", );
}

unsigned char RecPhoto ( unsigned char tmp ) {
    static unsigned char EndFlag = 0;
    static unsigned int i = 0;

    switch ( tmp ) {
        case 0xFF:
            EndFlag = 1;
            i = 1;
            break;

        default:
            if ( EndFlag == 1 && i == 2 ) {
                if ( tmp == 0xD9 ) {
                    EndFlag = 0;
                    i = 0;
                    return 0;
                }
            }

            break;
    }

    i++;
    return 1;
}