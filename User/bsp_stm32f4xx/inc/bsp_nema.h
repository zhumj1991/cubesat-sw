#ifndef _BSP_NEMA_H_
#define _BSP_NEMA_H_

#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"

__packed typedef struct
{
	u8 num;				//satellite number
	u8 eledeg;		//elevation in degrees
	u16 azideg;		//azimuth in degrees to true
	u8 sn;				//SNR in dB
}nmea_slmsg;

//UTC
__packed typedef struct
{
		u16 year;		
		u8 month;		
		u8 date;		
		u8 hour;		
		u8 min;			
		u8 sec;			
}nmea_utc_time;

//NEMA 0183
__packed typedef struct
{
		u8 svnum;								//satellites in view
		nmea_slmsg slmsg[12];		
		nmea_utc_time utc;			//UTC
		u32 latitude;						//latitude * 100000
		u8 nshemi;							//North or South
		u32 longitude;					//longitude * 100000
		u8 ewhemi;							//East or West
		u8 gpssta;							//GPS quality indicator
		u8 posslnum;						//Number of satellites in view, 00 - 12
		u8 possl[12];						
		u8 fixmode;							//mode
		u16 pdop;								//PDOP in meters
		u16 hdop;								//HDOP in meters
		u16 vdop;								//VDOP in meters
		int altitude;						//antenna altitude above/below mean-sea-level * 10
		u16 speed;							//speed * 1000
}nmea_msg;


void gps_analyze(nmea_msg gpsx, char *buf);

#endif
