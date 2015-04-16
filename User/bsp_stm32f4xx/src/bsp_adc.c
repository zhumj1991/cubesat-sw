#include "stm32f4xx.h"


/* define ---------------------------------------------------------------------*/
#define ADC1_DR_ADDRESS          ((uint32_t)0x4001204C)

/* ���� ----------------------------------------------------------------------*/
#ifndef	SAMPDEPTH
	#define	SAMPDEPTH			256
#endif
extern	uint16_t	ADCConvertedValue[SAMPDEPTH];

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitADC
*	����˵��: ADC��ʼ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitADC(void)
{
  ADC_InitTypeDef					ADC_InitStructure;
	ADC_CommonInitTypeDef		ADC_CommonInitStructure;
	DMA_InitTypeDef					DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef				GPIO_InitStructure;
	
	/* ʹ������ʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* DMA2 Stream0 channel0 ����-------------------------------------------------- */
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDRESS;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADCConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = SAMPDEPTH;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	
	/* DMA2_Stream0 ʹ�� */
	DMA_Cmd(DMA2_Stream0, ENABLE);
	
	DMA_ITConfig(DMA2_Stream0,DMA_IT_TC,ENABLE);				//ʹ��DMA2 Stream0�ж�
	/* NVIC configuration*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
    
	/****************************************************************************   
	  PPCLK2 = HCLK / 2 
	  ����ѡ�����2��Ƶ
	  ADCCLK = PCLK2 /8 = HCLK / 8 = 168 / 8 = 21M
    ADC����Ƶ�ʣ� Sampling Time + Conversion Time = 480 + 12 cycles = 492cyc
                  Conversion Time = 21MHz / 492cyc = 42.6ksps. 
	*****************************************************************************/
    
	/* ADC Common ���� ----------------------------------------------------------*/
	ADC_CommonInitStructure.ADC_Mode = ADC_TripleMode_RegSimult;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	/* ADC1 ����ͨ������ ---------------------------------------------------------*/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC1 ����ͨ��16���� -----------------------------------------------------------*/
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 2, ADC_SampleTime_480Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_15Cycles);  
	
	//ADC_ExternalTrigInjectedConvConfig(ADC1,ADC_ExternalTrigInjecConv_T2_TRGO);						//��ADC1�ⲿ����
	/* ADC1 ע��ͨ��10���� -----------------------------------------------------------*/
	//ADC_InjectedChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_15Cycles); 
	/* ʹ���¶ȼ�� */
	//ADC_TempSensorVrefintCmd(ENABLE); 
    
	/* ʹ�� ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);
    
	/* ������һ��ADC�����ʹ��DMA����(��ADCģʽ) */
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	
	/* ʹ�� ADC1 */
	ADC_Cmd(ADC1, ENABLE);
    
	/* �������ADC1ת�� */
	//ADC_SoftwareStartConv(ADC1);
}

/*
*************************************************************************************************
  VSENSE���¶ȴ������ĵ�ǰ�����ѹ����ADC_DR �Ĵ����еĽ��ADC_ConvertedValue֮���ת����ϵΪ�� 
            ADC_ConvertedValue * Vdd
  VSENSE = --------------------------
            Vdd_convert_value(0xFFF)  
    ADCת�������Ժ󣬶�ȡADC_DR�Ĵ����еĽ����ת���¶�ֵ���㹫ʽ���£�
          V25 - VSENSE
  T(��) = ------------  + 25
           Avg_Slope
  V25��  �¶ȴ�������25��ʱ �������ѹ������ֵ0.76 V��
  Avg_Slope���¶ȴ����������ѹ���¶ȵĹ�������������ֵ2.5 mV/�档
************************************************************************************************
*/
float GetTemp(uint16_t advalue)
{
    float Vtemp_sensor;
    float Current_Temp;
    
    Vtemp_sensor = advalue * 3.3/ 4095;  				           
    Current_Temp = (Vtemp_sensor - 0.76)/0.0025 + 25;  
    
    return Current_Temp;
}

/*-----------------------------------------
 ����˵��:ADC����DMA2ͨ��0�жϷ������
 		  DMA��ADֵ���䵽��������ɺ�رն�
		  ʱ��3(��Ϊ����ADת���Ķ�ʱ��)ͬʱ
		  �ø�����ɱ�־λΪ1,����ʱ��3��Ӧ
		  ���п���. 
------------------------------------------*/
void DMA2_Stream0_IRQHandler(void)
{ 
      DMA_ClearFlag(DMA2_Stream0, DMA_FLAG_TCIF0);	//���DMA��������ж�
      TIM_Cmd(TIM2,DISABLE);		//�ر�TIM3
}
/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
