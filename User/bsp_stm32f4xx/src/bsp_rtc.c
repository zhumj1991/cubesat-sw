#include "includes.h"


/*
*********************************************************************************************************
*	函 数 名: RTC_Config
*	功能说明: 用于配置时间戳功能
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t RTC_Config(void)
{
	RTC_InitTypeDef  RTC_InitStructure;	
	RTC_TimeTypeDef  RTC_TimeStructure;
	RTC_DateTypeDef  RTC_DateStructure;
	
	int ret;
	uint32_t timeout = 50;
	/* 用于设置RTC分频 */
	__IO uint32_t uwAsynchPrediv = 0;
	__IO uint32_t uwSynchPrediv = 0;
	
//	if(RCC_GetSYSCLKSource() == 0x00)
//		printf("Current system clock:HSI\r\n");
//	if(RCC_GetSYSCLKSource() == 0x04)
//		printf("Current system clock:HSE\r\n");
//	else if(RCC_GetSYSCLKSource() == 0x08)
//		printf("Current system clock:PLL\r\n");
	
		
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);	/* 使能PWR时钟 */	
	PWR_BackupAccessCmd(ENABLE);							/* 允许访问备份寄存器 */
	
	/* 选择LSE作为RTC时钟 */
	RCC_LSEConfig(RCC_LSE_ON);								/* 使能LSE振荡器  */	  
	while((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (timeout-- > 0)){	/* 等待就绪 */
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

		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);	/* 选择RTC时钟源 */
		ret = RTC_SOURCE_LSI;
		
	} else {
		/* LSE is OK */
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);	/* 选择RTC时钟源 */
		ret = RTC_SOURCE_LSE;
	}		
	RCC_RTCCLKCmd(ENABLE);										/* 使能RTC时钟 */
	RTC_WaitForSynchro();											/* 等待RTC APB寄存器同步 */

	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x9527) {
		RTC_WriteProtectionCmd(DISABLE);
    RTC_EnterInitMode();
		
		/* ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
		uwSynchPrediv = 0xFF;
		uwAsynchPrediv = 0x7F;

//		RTC_TimeStampCmd(RTC_TimeStampEdge_Falling, ENABLE);  /* 使能时间戳 */
//		RTC_TimeStampPinSelection(RTC_TamperPin_PC13);	

		/* 配置RTC数据寄存器和分频器  */
		RTC_InitStructure.RTC_AsynchPrediv = uwAsynchPrediv;
		RTC_InitStructure.RTC_SynchPrediv = uwSynchPrediv;
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;	
		RTC_Init(&RTC_InitStructure);						/* RTC初始化 */

		/* 设置年月日和星期 */
		RTC_DateStructure.RTC_Year = 0x15;
		RTC_DateStructure.RTC_Month = RTC_Month_April;
		RTC_DateStructure.RTC_Date = 0x05;
		RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Sunday;
		RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);

		/* 设置时分秒，以及显示格式 */
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
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);/* 得到时间 */
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);/* 得到日期 */
	
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
*	函 数 名: bsp_InitRTC
*	功能说明: 初始化RTC
*	形    参：无
*	返 回 值: 无		        
*********************************************************************************************************
*/
int8_t bsp_rtc_init(void)
{
	int ret = 0;
	/* RTC 配置  */
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

