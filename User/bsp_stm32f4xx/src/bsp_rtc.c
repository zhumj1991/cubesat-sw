#include "includes.h"


/*
*********************************************************************************************************
*	�� �� ��: RTC_Config
*	����˵��: ��������ʱ�������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint8_t RTC_Config(void)
{
	RTC_InitTypeDef  RTC_InitStructure;	
	RTC_TimeTypeDef  RTC_TimeStructure;
	RTC_DateTypeDef  RTC_DateStructure;
	
	int ret;
	uint32_t timeout = 50;
	/* ��������RTC��Ƶ */
	__IO uint32_t uwAsynchPrediv = 0;
	__IO uint32_t uwSynchPrediv = 0;
	
//	if(RCC_GetSYSCLKSource() == 0x00)
//		printf("Current system clock:HSI\r\n");
//	if(RCC_GetSYSCLKSource() == 0x04)
//		printf("Current system clock:HSE\r\n");
//	else if(RCC_GetSYSCLKSource() == 0x08)
//		printf("Current system clock:PLL\r\n");
	
		
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);	/* ʹ��PWRʱ�� */	
	PWR_BackupAccessCmd(ENABLE);							/* ������ʱ��ݼĴ��� */
	
	/* ѡ��LSE��ΪRTCʱ�� */
	RCC_LSEConfig(RCC_LSE_ON);								/* ʹ��LSE����  */	  
	while((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (timeout-- > 0)){	/* �ȴ����� */
		BSP_DelayUS(1);
	}
	if(!timeout) {
		/* select LSI as RTC clock source */
		timeout = 50;

		RCC_LSICmd(ENABLE);											/* Enable the LSI OSC */	

		while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET && (timeout-- > 0)){	/* Wait till LSI is ready */
			BSP_DelayUS(1);
		}

		if(!timeout) {
			return RTC_NO_SOURCE;
		}

		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);	/* ѡ��RTCʱ��Դ */
		ret = RTC_SOURCE_LSI;
		
	} else {
		/* LSE is OK */
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);	/* ѡ��RTCʱ��Դ */
		ret = RTC_SOURCE_LSE;
	}		
	RCC_RTCCLKCmd(ENABLE);										/* ʹ��RTCʱ�� */
	RTC_WaitForSynchro();											/* �ȴ�RTC APB�Ĵ���ͬ�� */

	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x9527) {
		RTC_WriteProtectionCmd(DISABLE);
    RTC_EnterInitMode();
		
		/* ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
		uwSynchPrediv = 0xFF;
		uwAsynchPrediv = 0x7F;

//		RTC_TimeStampCmd(RTC_TimeStampEdge_Falling, ENABLE);  /* ʹ��ʱ��� */
//		RTC_TimeStampPinSelection(RTC_TamperPin_PC13);	

		/* ����RTC���ݼĴ����ͷ�Ƶ��  */
		RTC_InitStructure.RTC_AsynchPrediv = uwAsynchPrediv;
		RTC_InitStructure.RTC_SynchPrediv = uwSynchPrediv;
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;	
		RTC_Init(&RTC_InitStructure);						/* RTC��ʼ�� */

		/* ���������պ����� */
		RTC_DateStructure.RTC_Year = 0x15;
		RTC_DateStructure.RTC_Month = RTC_Month_April;
		RTC_DateStructure.RTC_Date = 0x05;
		RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Sunday;
		RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);

		/* ����ʱ���룬�Լ���ʾ��ʽ */
		RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
		RTC_TimeStructure.RTC_Hours   = 0x00;
		RTC_TimeStructure.RTC_Minutes = 0x00;
		RTC_TimeStructure.RTC_Seconds = 0x00; 
		RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);

		RTC_ExitInitMode();
    RTC_WriteBackupRegister(RTC_BKP_DR0,0x9527);
    RTC_WriteProtectionCmd(ENABLE);
    RTC_WriteBackupRegister(RTC_BKP_DR0,0x9527);
	}
	PWR_BackupAccessCmd(DISABLE);
	
	return ret;
}


void rtc_get(struct rtc_time *tm)
{
	unsigned int have_retried = 0;
	RTC_TimeTypeDef  RTC_TimeStructure;
	RTC_DateTypeDef  RTC_DateStructure;

retry_get_time:	
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);/* �õ�ʱ�� */
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);/* �õ����� */
	
	tm->tm_sec = RTC_TimeStructure.RTC_Seconds;
	tm->tm_min = RTC_TimeStructure.RTC_Minutes;
	tm->tm_hour = RTC_TimeStructure.RTC_Hours;

	/* the only way to work out wether the system was mid-update
	 * when we read it is to check the second counter, and if it
	 * is zero, then we re-try the entire read
	 */
	if (tm->tm_sec == 0 && !have_retried) {
		have_retried = 1;
		goto retry_get_time;
	}	
	
	tm->tm_wday = RTC_DateStructure.RTC_WeekDay;
	tm->tm_mday = RTC_DateStructure.RTC_Date;
	tm->tm_mon = RTC_DateStructure.RTC_Month;
	tm->tm_year = RTC_DateStructure.RTC_Year;

}

void rtc_set(struct rtc_time *tm)
{
	RTC_TimeTypeDef  RTC_TimeStructure;
	RTC_DateTypeDef  RTC_DateStructure;

	RTC_TimeStructure.RTC_H12 = RTC_H12_AM;
	RTC_TimeStructure.RTC_Seconds = tm->tm_sec;
	RTC_TimeStructure.RTC_Minutes = tm->tm_min;
	RTC_TimeStructure.RTC_Hours = tm->tm_hour;
	
	RTC_DateStructure.RTC_WeekDay = tm->tm_wday;
	RTC_DateStructure.RTC_Date = tm->tm_mday;
	RTC_DateStructure.RTC_Month = tm->tm_mon;
	RTC_DateStructure.RTC_Year = tm->tm_year;

	PWR_BackupAccessCmd(ENABLE);	
	RTC_WriteProtectionCmd(DISABLE);
	RTC_EnterInitMode();
	
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
	
	RTC_ExitInitMode();
	RTC_WriteProtectionCmd(ENABLE);
	PWR_BackupAccessCmd(DISABLE);
}

void rtc_reset(void)
{
	return;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitRTC
*	����˵��: ��ʼ��RTC
*	��    �Σ���
*	�� �� ֵ: ��		        
*********************************************************************************************************
*/
int8_t bsp_rtc_init(void)
{
	int ret = 0;
	/* RTC ����  */
	ret = RTC_Config();
	
	switch(ret) {
		case 1:
			debug("RTC clock sourcr: LSE\r\n");
			break;
		case 2:
			debug("RTC clock sourcr: LSI\r\n");
			break;
		case 3:
			ret = -2;
			debug("RTC clock sourcr: null\r\n");
			break;
		default:
			ret = -1;
			break;
	}
	
	return ret;
}

