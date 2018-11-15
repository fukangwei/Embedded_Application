#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "stdlib.h"
#include "string.h"
#include "cJSON.h"

char text[] = "{\"timestamp\":\"2013-11-19T08:50:11\",\"value\":1}"; /* ��������JSON���ݰ� */

int main ( void ) {
    cJSON *json = NULL, *json_value = NULL, *json_timestamp = NULL;
    SystemInit();
    delay_init ( 72 );
    NVIC_Configuration();
    uart_init ( 9600 );
    LED_Init();
    json = cJSON_Parse ( text ); /* �������ݰ� */

    if ( !json ) {
        printf ( "Error before: [%s]\n", cJSON_GetErrorPtr() );
    } else {
        json_value = cJSON_GetObjectItem ( json , "value" ); /* ��������ֵ */

        if ( json_value->type == cJSON_Number ) {
            printf ( "value:%d\r\n", json_value->valueint ); /* ��valueint�л�ý�� */
        }

        json_timestamp = cJSON_GetObjectItem ( json , "timestamp" ); /* ����ʱ��� */

        if ( json_timestamp->type == cJSON_String ) {
            printf ( "%s\r\n", json_timestamp->valuestring ); /* valuestring�л�ý�� */
        }

        cJSON_Delete ( json ); /* �ͷ��ڴ�ռ� */
    }

    while ( 1 ) {
        LED0 = !LED0;
        delay_ms ( 500 );
    }
}