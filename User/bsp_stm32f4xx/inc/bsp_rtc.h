#ifndef __BSP_RTC_H
#define __BSP_RTC_H

#include	<stm32f4xx.h>

#define	RTC_SOURCE_LSE			1
#define	RTC_SOURCE_LSI			2
#define	RTC_NO_SOURCE				3

__packed struct rtc_time {
	uint8_t tm_sec;
	uint8_t tm_min;
	uint8_t tm_hour;
	uint8_t tm_mday;
	uint8_t tm_mon;
	uint16_t tm_year;
	uint8_t tm_wday;
	uint8_t tm_yday;
	uint8_t tm_isdst;
};


int8_t bsp_rtc_init(void);
void rtc_get(struct rtc_time *tm);
void rtc_set(struct rtc_time *tm);
void rtc_reset(void);

#endif
