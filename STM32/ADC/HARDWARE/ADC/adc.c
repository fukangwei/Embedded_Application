#include "adc.h"

void Adc_Init ( void ) {
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE );
    RCC_ADCCLKConfig ( RCC_PCLK2_Div6 );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  /* 模拟输入引脚 */
    GPIO_Init ( GPIOA, &GPIO_InitStructure );
    ADC_DeInit ( ADC1 ); /* 将外设ADC1的全部寄存器重设为缺省值 */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  /* ADC工作模式：ADC1和ADC2工作在独立模式 */
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; /* 转换由软件而不是外部触发启动 */
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; /* ADC数据右对齐 */
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init ( ADC1, &ADC_InitStructure ); /* 根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器 */
    ADC_Cmd ( ADC1, ENABLE ); /* 使能指定的ADC1 */
    ADC_ResetCalibration ( ADC1 ); /* 重置指定的ADC1的校准寄存器 */

    while ( ADC_GetResetCalibrationStatus ( ADC1 ) );

    ADC_StartCalibration ( ADC1 ); /* 开始指定ADC1的校准状态 */

    while ( ADC_GetCalibrationStatus ( ADC1 ) );

    ADC_SoftwareStartConvCmd ( ADC1, ENABLE );
}

u16 Get_Adc ( u8 ch ) { /* 获得ADC值 */
    /* 设置指定ADC的规则组通道，设置它们的转化顺序和采样时间 */
    ADC_RegularChannelConfig ( ADC1, ch, 1, ADC_SampleTime_239Cycles5 ); /* ADC1，ADC通道ch，规则采样顺序值为1，采样时间为239.5周期 */
    ADC_SoftwareStartConvCmd ( ADC1, ENABLE );

    while ( !ADC_GetFlagStatus ( ADC1, ADC_FLAG_EOC ) ); /* 等待转换结束 */

    return ADC_GetConversionValue ( ADC1 ); /* 返回最近一次ADC1规则组的转换结果 */
}