#include "bsp.h"


uint8_t nema_comma_pos(char *buf, char cx)
{	 		    
	char *p = buf;

	while(cx)
	{		 
		if((*buf == '*') || (*buf < ' ') || (*buf > 'z'))
			return 0xFF;
		
		if(*buf == ',') cx--;
		buf++;
	}
	return (buf - p); 
}

/*
 * m ^ n
 */
uint32_t nema_pow(uint8_t m, uint8_t n)
{
	u32 result = 1;	
	
	while(n--)
		result *= m;    
	
	return result;
}

/*
 * str -> num from the first str to the ','
 * buf: pointer of str; dx: length of fractional part
 */
int nema_str2num(char *buf, char *dx)
{
	char *p = buf;
	uint32_t ires = 0, fres = 0;
	uint8_t ilen = 0, flen = 0, i;
	uint8_t mask = 0;
	int res;

	while(1) {
		if (*p == '-') {
			mask |= 0x02;
			p++;
		}
		if (*p == ','||(*p == '*'))
			break;
		if(*p == '.') {
			mask |= 0x01;
			p++;
		} else if ((*p > '9') || (*p < '0')) {	
			ilen = 0;
			flen = 0;
			break;
		}	
		if (mask & 0x01)
			flen++;
		else
			ilen++;
		p++;
	}
	
	if (mask & 0x02)
		buf++;
	
	for(i = 0; i < ilen; i++)
		ires += nema_pow(10, ilen - 1 - i) * (buf[i] - '0');

	if (flen > 5)
		flen = 5;	

	*dx = flen;	 		

	for(i = 0; i < flen; i++) 
		fres += nema_pow(10, flen - 1 - i) * (buf[ilen + 1 + i] - '0');

	res = ires * nema_pow(10, flen) + fres;

	if(mask & 0x02)
		res = -res;		   

	return res;
}



/*
	$GPGGA
	例：$GPGGA,092204.999,4250.5589,S,14718.5084,E,1,04,24.4,19.7,M,,,,0000*1F
	字段0：$GPGGA，语句ID，表明该语句为Global Positioning System Fix Data（GGA）GPS定位信息
	字段1：UTC 时间，hhmmss.sss，时分秒格式
	字段2：纬度ddmm.mmmm，度分格式（前导位数不足则补0）
	字段3：纬度N（北纬）或S（南纬）
	字段4：经度dddmm.mmmm，度分格式（前导位数不足则补0）
	字段5：经度E（东经）或W（西经）
	字段6：GPS状态，0=未定位，1=非差分定位，2=差分定位，3=无效PPS，6=正在估算
	字段7：正在使用的卫星数量（00 - 12）（前导位数不足则补0）
	字段8：HDOP水平精度因子（0.5 - 99.9）
	字段9：海拔高度（-9999.9 - 99999.9）
	字段10：地球椭球面相对大地水准面的高度
	字段11：差分时间（从最近一次接收到差分信号开始的秒数，如果不是差分定位将为空）
	字段12：差分站ID号0000 - 1023（前导位数不足则补0，如果不是差分定位将为空）
	字段13：校验值
*/
void gps_GPGGA(nmea_msg *gpsx, char *buf)
{
	char *p, dx;			 
	uint8_t posx;    

	p = buf;
	posx = nema_comma_pos(p, 6);						
	if(posx != 0xFF)
		gpsx->gpssta = nema_str2num(p + posx, &dx);

	posx = nema_comma_pos(buf, 7);
	if(posx != 0xFF)
		gpsx->posslnum = nema_str2num(p + posx, &dx);

	posx = nema_comma_pos(buf, 9);							
	if(posx != 0xFF)
		gpsx->altitude = nema_str2num(p + posx, &dx); 
}

/*
	$GPGSA
	例：$GPGSA,A,3,01,20,19,13,,,,,,,,,40.4,24.4,32.2*0A
	字段0：$GPGSA，语句ID，表明该语句为GPS DOP and Active Satellites（GSA）当前卫星信息
	字段1：定位模式，A=自动手动2D/3D，M=手动2D/3D
	字段2：定位类型，1=未定位，2=2D定位，3=3D定位
	字段3：PRN码（伪随机噪声码），第1信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
	字段4：PRN码（伪随机噪声码），第2信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
	字段5：PRN码（伪随机噪声码），第3信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
	字段6：PRN码（伪随机噪声码），第4信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
	字段7：PRN码（伪随机噪声码），第5信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
	字段8：PRN码（伪随机噪声码），第6信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
	字段9：PRN码（伪随机噪声码），第7信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
	字段10：PRN码（伪随机噪声码），第8信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
	字段11：PRN码（伪随机噪声码），第9信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
	字段12：PRN码（伪随机噪声码），第10信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
	字段13：PRN码（伪随机噪声码），第11信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
	字段14：PRN码（伪随机噪声码），第12信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
	字段15：PDOP综合位置精度因子（0.5 - 99.9）
	字段16：HDOP水平精度因子（0.5 - 99.9）
	字段17：VDOP垂直精度因子（0.5 - 99.9）
	字段18：校验值
*/
void gps_GPGSA(nmea_msg *gpsx, char *buf)
{
	char *p,dx;			 
	uint8_t posx; 
	uint8_t i;   

	p = buf;
	
	posx = nema_comma_pos(p, 2);						
	if(posx != 0xFF)
		gpsx->fixmode = nema_str2num(p + posx, &dx);	
	
	for(i = 0; i < 12; i++)							
	{
		posx=nema_comma_pos(p, 3 + i);					 
		if(posx != 0xFF)
			gpsx->possl[i] = nema_str2num(p + posx, &dx);
		else
			break; 
	}
	
	posx = nema_comma_pos(p, 15);							
	if(posx != 0xFF)
		gpsx->pdop = nema_str2num(p + posx, &dx); 
	
	posx=nema_comma_pos(p,16);								
	if(posx != 0xFF)
		gpsx->hdop = nema_str2num(p + posx, &dx);  

	posx = nema_comma_pos(p, 17);
	if(posx != 0xFF)
		gpsx->vdop=nema_str2num(p + posx,& dx); 
}
/*
	$GPGSV
	例：$GPGSV,3,1,10,20,78,331,45,01,59,235,47,22,41,069,,13,32,252,45*70
			$GPGSV,2,1,07,07,79,048,42,02,51,062,43,26,36,256,42,27,27,138,42*71
			$GPGSV,2,2,07,09,23,313,42,04,19,159,41,15,12,041,42*41

	字段0：$GPGSV，语句ID，表明该语句为GPS Satellites in View（GSV）可见卫星信息
	字段1：本次GSV语句的总数目（1 - 3）
	字段2：本条GSV语句是本次GSV语句的第几条（1 - 3）
	字段3：当前可见卫星总数（00 - 12）（前导位数不足则补0）

	字段4：PRN 码（伪随机噪声码）（01 - 32）（前导位数不足则补0）
	字段5：卫星仰角（00 - 90）度（前导位数不足则补0）
	字段6：卫星方位角（00 - 359）度（前导位数不足则补0）
	字段7：信噪比（00－99）dbHz

	字段8：PRN 码（伪随机噪声码）（01 - 32）（前导位数不足则补0）
	字段9：卫星仰角（00 - 90）度（前导位数不足则补0）
	字段10：卫星方位角（00 - 359）度（前导位数不足则补0）
	字段11：信噪比（00－99）dbHz

	字段12：PRN 码（伪随机噪声码）（01 - 32）（前导位数不足则补0）
	字段13：卫星仰角（00 - 90）度（前导位数不足则补0）
	字段14：卫星方位角（00 - 359）度（前导位数不足则补0）
	字段15：信噪比（00－99）dbHz
	字段16：校验值
*/
void gps_GPGSV(nmea_msg *gpsx, char *buf)
{
	char *p, dx;
	uint8_t num, i;
	uint8_t posx;   	 

	p = buf;
	num = p[9] - '0';
	
	posx = nema_comma_pos(p, 3); 			
	if(posx != 0xFF)
		gpsx->svnum = nema_str2num(p + posx, &dx);
	
	 
	for(i = 0; i < 4; i++)
	{	  
		posx = nema_comma_pos(p, 4 + i*4);
		if(posx != 0xFF)
			gpsx->slmsg[(num - 1) * 4 + i].num = nema_str2num(p + posx, &dx);
		else
			break; 
		
		posx = nema_comma_pos(p, 5 + i*4);
		if(posx != 0xFF)
			gpsx->slmsg[(num - 1) * 4 + i].eledeg = nema_str2num(p + posx, &dx);
		else
			break;
		
		posx = nema_comma_pos(p,6 + i*4);
		if(posx != 0xFF)
			gpsx->slmsg[(num - 1) * 4 + i].azideg=nema_str2num(p + posx, &dx);
		else
			break; 
		
		posx = nema_comma_pos(p, 7 + i*4);
		if(posx != 0xFF)
			gpsx->slmsg[(num - 1) * 4 + i].sn=nema_str2num(p + posx, &dx);	
		else
			break;   
	}   

}

/*
	$GPRMC
	例：$GPRMC,024813.640,A,3158.4608,N,11848.3737,E,10.05,324.27,150706,,,A*50
	字段0：$GPRMC，语句ID，表明该语句为Recommended Minimum Specific GPS/TRANSIT Data（RMC）推荐最小定位信息
	字段1：UTC时间，hhmmss.sss格式
	字段2：状态，A=定位，V=未定位
	字段3：纬度ddmm.mmmm，度分格式（前导位数不足则补0）
	字段4：纬度N（北纬）或S（南纬）
	字段5：经度dddmm.mmmm，度分格式（前导位数不足则补0）
	字段6：经度E（东经）或W（西经）
	字段7：速度，节，Knots
	字段8：方位角，度
	字段9：UTC日期，DDMMYY格式
	字段10：磁偏角，（000 - 180）度（前导位数不足则补0）
	字段11：磁偏角方向，E=东W=西
	字段16：校验值
*/
void gps_GPRMC(nmea_msg *gpsx, char *buf)
{
	char *p, dx;			 
	uint8_t posx;     
	uint32_t temp;	   
	float rs; 
	
	p = buf;
	
	posx=nema_comma_pos(p, 1);								
	if(posx != 0xFF)
	{
		temp = nema_str2num(p + posx, &dx) / nema_pow(10, dx);	 
		gpsx->utc.hour = temp / 10000;
		gpsx->utc.min = (temp / 100) % 100;
		gpsx->utc.sec = temp % 100;	 	 
	}	
	
	posx = nema_comma_pos(p, 3);
	if(posx != 0xFF)
	{
		temp = nema_str2num(p + posx, &dx);		 	 
		gpsx->latitude = temp / nema_pow(10, dx + 2);	
		rs = temp % nema_pow(10, dx + 2);			
		gpsx->latitude = gpsx->latitude * nema_pow(10, 5) + (rs * nema_pow(10, 5 - dx)) / 60;
	}
	
	posx = nema_comma_pos(p, 4);							
	if(posx != 0xFF)
		gpsx->nshemi = *(p + posx);
	
 	posx = nema_comma_pos(p, 5);								
	if(posx != 0xFF)
	{												  
		temp = nema_str2num(p + posx, &dx);		 	 
		gpsx->longitude = temp / nema_pow(10, dx + 2);
		rs = temp % nema_pow(10, dx + 2);					 
		gpsx->longitude = gpsx->longitude * nema_pow(10, 5) + (rs * nema_pow(10, 5 - dx)) / 60;
	}
	posx=nema_comma_pos(p, 6);						
	if(posx != 0xFF)
		gpsx->ewhemi = *(p + posx);	
	
	posx=nema_comma_pos(p, 9);							
	if(posx != 0xFF)
	{
		temp=nema_str2num(p + posx, &dx);		 			
		gpsx->utc.date=temp / 10000;
		gpsx->utc.month=(temp / 100) % 100;
		gpsx->utc.year=2000 + temp % 100;	 	 
	}
}

/*
	$GPVTG
	例：$GPVTG,89.68,T,,M,0.00,N,0.0,K*5F
	字段0：$GPVTG，语句ID，表明该语句为Track Made Good and Ground Speed（VTG）地面速度信息
	字段1：运动角度，000 - 359，（前导位数不足则补0）
	字段2：T=真北参照系
	字段3：运动角度，000 - 359，（前导位数不足则补0）
	字段4：M=磁北参照系
	字段5：水平运动速度（0.00）（前导位数不足则补0）
	字段6：N=节，Knots
	字段7：水平运动速度（0.00）（前导位数不足则补0）
	字段8：K=公里/时，km/h
	字段9：校验值
*/
void gps_GPVTG(nmea_msg *gpsx, char *buf)
{
	char *p, dx;			 
	uint8_t posx;    

	p=buf;
	
	posx = nema_comma_pos(p, 7);							
	if(posx != 0xFF)
	{
		gpsx->speed = nema_str2num(p + posx, &dx);
		
		if(dx < 3)
			gpsx->speed *= nema_pow(10, 3 - dx);	 	 		
	}
}

/*
	$GPGLL
	例：$GPGLL,4250.5589,S,14718.5084,E,092204.999,A*2D
	字段0：$GPGLL，语句ID，表明该语句为Geographic Position（GLL）地理定位信息
	字段1：纬度ddmm.mmmm，度分格式（前导位数不足则补0）
	字段2：纬度N（北纬）或S（南纬）
	字段3：经度dddmm.mmmm，度分格式（前导位数不足则补0）
	字段4：经度E（东经）或W（西经）
	字段5：UTC时间，hhmmss.sss格式
	字段6：状态，A=定位，V=未定位
	字段7：校验值
*/
void gps_GPGLL(nmea_msg *gpsx, char *buf)
{

}

/*
 * NEMA-0183数据包解析
 */
void gps_analyze(nmea_msg gpsx, char *buf)
{
	if (memcmp(buf, "GPGGA,", 6) == 0)
	{
		gps_GPGGA(&gpsx, buf);
	}
	else if (memcmp(buf, "GPGSA,", 6) == 0)
	{
		gps_GPGSA(&gpsx, buf);
	}
	else if (memcmp(buf, "GPGSV,", 6) == 0)
	{
		gps_GPGSV(&gpsx, buf);
	}
	else if (memcmp(buf, "GPRMC,", 6) == 0)
	{
		gps_GPRMC(&gpsx, buf);
	}
	else if (memcmp(buf, "GPVTG,", 6) == 0)
	{
		gps_GPVTG(&gpsx, buf);
	}
	else if (memcmp(buf, "GPGLL,", 6) == 0)
	{
		gps_GPGLL(&gpsx, buf);
	}
}
