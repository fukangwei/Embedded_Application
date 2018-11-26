#include <stddef.h>
//#include "GUI.h"
#include "DIALOG.h"
//#include "WM.h"
/*#include "BUTTON.h"
#include "CHECKBOX.h"
#include "DROPDOWN.h"
//#include "EDIT.h"
/*#include "FRAMEWIN.h"
#include "LISTBOX.h"
#include "MULTIEDIT.h"
#include "RADIO.h"
#include "SLIDER.h"
#include "TEXT.h"
#include "PROGBAR.h"
#include "SCROLLBAR.h"
#include "LISTVIEW.h"
#include <stdlib.h>*/
extern const GUI_FONT GUI_FontHZ_Song_16;
extern const GUI_FONT GUI_FontHZ_Song_12;

static int createFlag = 0, createFlag1, i = 10, t = 555;
static char m;
static WM_HWIN _hSet, _hOver, text0, text1, text2, text3, text4, _hClient, _hWinControl, _hClient1;
#define TEXT_ID_Client0 (GUI_ID_USER + 250)
static WM_CALLBACK *_pcbCallbackButton;

unsigned char ADC_STR2[5];
unsigned short int ADC_ConvertedValue[2];

static void _cbButton ( WM_MESSAGE *pMsg ) { /* 设置按键风格函数 */
    char dataBuffer[10];
    GUI_RECT Rect;
    WM_HWIN hWin;
    hWin = pMsg->hWin;

    switch ( pMsg->MsgId ) {
        case WM_PAINT:
            WM_GetClientRect ( &Rect );
            GUI_SetColor ( GUI_WHITE );
            GUI_DrawRectEx ( &Rect );

            if ( BUTTON_IsPressed ( hWin ) ) {
                GUI_DrawRect ( 1, 1, Rect.x1 - 1, Rect.y1 - 1 );
            }

            GUI_SetColor ( GUI_BLACK );
            GUI_SetFont ( &GUI_FontHZ_Song_12 );
            GUI_SetTextMode ( GUI_TM_TRANS );
            BUTTON_GetText ( hWin, dataBuffer, sizeof ( dataBuffer ) );
            GUI_DispStringInRect ( dataBuffer, &Rect, GUI_TA_HCENTER | GUI_TA_VCENTER );
            break;

        default:
            _pcbCallbackButton ( pMsg );
    }
}