#include "ssi_cgi_handle.h"
#include "httpd.h"
#include "string.h"
#include "usart.h"
#include "stdio.h"
#include "fs.h"

#define INDEX_PAGE_SET_CGI_RSP_URL    "/index.html"
#define RESPONSE_PAGE_SET_CGI_RSP_URL "/response.ssi"

#define NUM_CONFIG_CGI_URIS (sizeof(ppcURLs ) / sizeof(tCGI))
#define NUM_CONFIG_SSI_TAGS  (sizeof(ppcTags) / sizeof (char *))

static char *LED_RED_CGIHandler ( int iIndex, int iNumParams, char *pcParam[], char *pcValue[] );
static char *LED_GREEN_CGIHandler ( int iIndex, int iNumParams, char *pcParam[], char *pcValue[] );
static char *Orther_CGIHandler ( int iIndex, int iNumParams, char *pcParam[], char *pcValue[] );
static int SSIHandler ( int iIndex, char *pcInsert, int iInsertLen );

static const tCGI ppcURLs[] = {
    { "/led_red.cgi",   LED_RED_CGIHandler },
    { "/led_green.cgi", LED_GREEN_CGIHandler },
    { "/orther.cgi",    Orther_CGIHandler },
};

static const char *ppcTags[] = {
    "onetree",
    "filest"
};

enum ssi_index_s {
    SSI_INDEX_ONETREE_GET = 0,
    SSI_INDEX_FILEST_GET
} ;

void init_ssi_cgi ( void ) {
    http_set_cgi_handlers ( ppcURLs , NUM_CONFIG_CGI_URIS );
    http_set_ssi_handler ( SSIHandler, ppcTags, NUM_CONFIG_SSI_TAGS );
}

//****************************************************************************
// This CGI handler is called whenever the web browser requests iocontrol.cgi
//****************************************************************************
static int FindCGIParameter ( const char *pcToFind, char *pcParam[], int iNumParams ) {
    int iLoop;

    for ( iLoop = 0; iLoop < iNumParams; iLoop++ ) {
        if ( strcmp ( pcToFind, pcParam[iLoop] ) == 0 ) {
            return ( iLoop );
        }
    }

    return ( -1 );
}

void  clear_response_bufer ( unsigned char *buffer ) {
    memset ( buffer, 0, strlen ( ( const char * ) buffer ) );
}
int num = 100;


static char *LED_RED_CGIHandler ( int iIndex, int iNumParams, char *pcParam[], char *pcValue[] ) {
    int  index;
    index = FindCGIParameter ( "red", pcParam, iNumParams );

    if ( index != -1 ) {
        clear_response_bufer ( data_response_buf );
        strcat ( ( char * ) ( data_response_buf ), "/img/black.gif" );
    }

    return RESPONSE_PAGE_SET_CGI_RSP_URL;
}

static char *LED_GREEN_CGIHandler ( int iIndex, int iNumParams, char *pcParam[], char *pcValue[] ) {
    int  index;
    index = FindCGIParameter ( "green", pcParam, iNumParams );

    if ( index != -1 ) {
        clear_response_bufer ( data_response_buf );
        strcat ( ( char * ) ( data_response_buf ), "/img/black.gif" );
    }

    return RESPONSE_PAGE_SET_CGI_RSP_URL;
}

static char *Orther_CGIHandler ( int iIndex, int iNumParams, char *pcParam[], char *pcValue[] ) {
    u8 buf[20];
    clear_response_bufer ( data_response_buf );
    strcat ( ( char * ) ( data_response_buf ), ";" );
    strcat ( ( char * ) ( data_response_buf ), buf );
    return RESPONSE_PAGE_SET_CGI_RSP_URL;
}


//*****************************************************************************
// This function is called by the HTTP server whenever it encounters an SSI
// tag in a web page.  The iIndex parameter provides the index of the tag in
// the ppcTags array. This function writes the substitution text
// into the pcInsert array, writing no more than iInsertLen characters.
//*****************************************************************************
static int SSIHandler ( int iIndex, char *pcInsert, int iInsertLen ) {
    switch ( iIndex ) {
        case SSI_INDEX_ONETREE_GET:
            break;

        case SSI_INDEX_FILEST_GET:
            break;

        default:
            strcpy ( pcInsert , "??" );
    }

    return strlen ( pcInsert );
}