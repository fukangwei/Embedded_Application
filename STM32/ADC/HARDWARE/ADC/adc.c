#include "adc.h"

void Adc_Init ( void ) {
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE );
    RCC_ADCCLKConfig ( RCC_PCLK2_Div6 );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  /* ģ���������� */
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    ADC_DeInit ( ADC1 ); /* ������ADC1��ȫ���Ĵ�������Ϊȱʡֵ */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  /* ADC����ģʽ��ADC1��ADC2�����ڶ���ģʽ */
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; /* ת��������������ⲿ�������� */
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; /* ADC�����Ҷ��� */
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init ( ADC1, &ADC_InitStructure ); /* ����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ��� */
    ADC_Cmd ( ADC1, ENABLE ); /* ʹ��ָ����ADC1 */
    ADC_ResetCalibration ( ADC1 ); /* ����ָ����ADC1��У׼�Ĵ��� */

    while ( ADC_GetResetCalibrationStatus ( ADC1 ) );

    ADC_StartCalibration ( ADC1 ); /* ��ʼָ��ADC1��У׼״̬ */

    while ( ADC_GetCalibrationStatus ( ADC1 ) );

    ADC_SoftwareStartConvCmd ( ADC1, ENABLE );
}

u16 Get_Adc ( u8 ch ) { /* ���ADCֵ */
    /* ����ָ��ADC�Ĺ�����ͨ�����������ǵ�ת��˳��Ͳ���ʱ�� */
    ADC_RegularChannelConfig ( ADC1, ch, 1, ADC_SampleTime_239Cycles5 ); /* ADC1��ADCͨ��ch���������˳��ֵΪ1������ʱ��Ϊ239.5���� */
    ADC_SoftwareStartConvCmd ( ADC1, ENABLE );

    while ( !ADC_GetFlagStatus ( ADC1, ADC_FLAG_EOC ) ); /* �ȴ�ת������ */

    return ADC_GetConversionValue ( ADC1 ); /* �������һ��ADC1�������ת����� */
}