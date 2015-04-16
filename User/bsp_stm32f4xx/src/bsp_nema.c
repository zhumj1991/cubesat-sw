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
	����$GPGGA,092204.999,4250.5589,S,14718.5084,E,1,04,24.4,19.7,M,,,,0000*1F
	�ֶ�0��$GPGGA�����ID�����������ΪGlobal Positioning System Fix Data��GGA��GPS��λ��Ϣ
	�ֶ�1��UTC ʱ�䣬hhmmss.sss��ʱ�����ʽ
	�ֶ�2��γ��ddmm.mmmm���ȷָ�ʽ��ǰ��λ��������0��
	�ֶ�3��γ��N����γ����S����γ��
	�ֶ�4������dddmm.mmmm���ȷָ�ʽ��ǰ��λ��������0��
	�ֶ�5������E����������W��������
	�ֶ�6��GPS״̬��0=δ��λ��1=�ǲ�ֶ�λ��2=��ֶ�λ��3=��ЧPPS��6=���ڹ���
	�ֶ�7������ʹ�õ�����������00 - 12����ǰ��λ��������0��
	�ֶ�8��HDOPˮƽ�������ӣ�0.5 - 99.9��
	�ֶ�9�����θ߶ȣ�-9999.9 - 99999.9��
	�ֶ�10��������������Դ��ˮ׼��ĸ߶�
	�ֶ�11�����ʱ�䣨�����һ�ν��յ�����źſ�ʼ��������������ǲ�ֶ�λ��Ϊ�գ�
	�ֶ�12�����վID��0000 - 1023��ǰ��λ��������0��������ǲ�ֶ�λ��Ϊ�գ�
	�ֶ�13��У��ֵ
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
	����$GPGSA,A,3,01,20,19,13,,,,,,,,,40.4,24.4,32.2*0A
	�ֶ�0��$GPGSA�����ID�����������ΪGPS DOP and Active Satellites��GSA����ǰ������Ϣ
	�ֶ�1����λģʽ��A=�Զ��ֶ�2D/3D��M=�ֶ�2D/3D
	�ֶ�2����λ���ͣ�1=δ��λ��2=2D��λ��3=3D��λ
	�ֶ�3��PRN�루α��������룩����1�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
	�ֶ�4��PRN�루α��������룩����2�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
	�ֶ�5��PRN�루α��������룩����3�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
	�ֶ�6��PRN�루α��������룩����4�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
	�ֶ�7��PRN�루α��������룩����5�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
	�ֶ�8��PRN�루α��������룩����6�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
	�ֶ�9��PRN�루α��������룩����7�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
	�ֶ�10��PRN�루α��������룩����8�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
	�ֶ�11��PRN�루α��������룩����9�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
	�ֶ�12��PRN�루α��������룩����10�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
	�ֶ�13��PRN�루α��������룩����11�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
	�ֶ�14��PRN�루α��������룩����12�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
	�ֶ�15��PDOP�ۺ�λ�þ������ӣ�0.5 - 99.9��
	�ֶ�16��HDOPˮƽ�������ӣ�0.5 - 99.9��
	�ֶ�17��VDOP��ֱ�������ӣ�0.5 - 99.9��
	�ֶ�18��У��ֵ
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
	����$GPGSV,3,1,10,20,78,331,45,01,59,235,47,22,41,069,,13,32,252,45*70
			$GPGSV,2,1,07,07,79,048,42,02,51,062,43,26,36,256,42,27,27,138,42*71
			$GPGSV,2,2,07,09,23,313,42,04,19,159,41,15,12,041,42*41

	�ֶ�0��$GPGSV�����ID�����������ΪGPS Satellites in View��GSV���ɼ�������Ϣ
	�ֶ�1������GSV��������Ŀ��1 - 3��
	�ֶ�2������GSV����Ǳ���GSV���ĵڼ�����1 - 3��
	�ֶ�3����ǰ�ɼ�����������00 - 12����ǰ��λ��������0��

	�ֶ�4��PRN �루α��������룩��01 - 32����ǰ��λ��������0��
	�ֶ�5���������ǣ�00 - 90���ȣ�ǰ��λ��������0��
	�ֶ�6�����Ƿ�λ�ǣ�00 - 359���ȣ�ǰ��λ��������0��
	�ֶ�7������ȣ�00��99��dbHz

	�ֶ�8��PRN �루α��������룩��01 - 32����ǰ��λ��������0��
	�ֶ�9���������ǣ�00 - 90���ȣ�ǰ��λ��������0��
	�ֶ�10�����Ƿ�λ�ǣ�00 - 359���ȣ�ǰ��λ��������0��
	�ֶ�11������ȣ�00��99��dbHz

	�ֶ�12��PRN �루α��������룩��01 - 32����ǰ��λ��������0��
	�ֶ�13���������ǣ�00 - 90���ȣ�ǰ��λ��������0��
	�ֶ�14�����Ƿ�λ�ǣ�00 - 359���ȣ�ǰ��λ��������0��
	�ֶ�15������ȣ�00��99��dbHz
	�ֶ�16��У��ֵ
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
	����$GPRMC,024813.640,A,3158.4608,N,11848.3737,E,10.05,324.27,150706,,,A*50
	�ֶ�0��$GPRMC�����ID�����������ΪRecommended Minimum Specific GPS/TRANSIT Data��RMC���Ƽ���С��λ��Ϣ
	�ֶ�1��UTCʱ�䣬hhmmss.sss��ʽ
	�ֶ�2��״̬��A=��λ��V=δ��λ
	�ֶ�3��γ��ddmm.mmmm���ȷָ�ʽ��ǰ��λ��������0��
	�ֶ�4��γ��N����γ����S����γ��
	�ֶ�5������dddmm.mmmm���ȷָ�ʽ��ǰ��λ��������0��
	�ֶ�6������E����������W��������
	�ֶ�7���ٶȣ��ڣ�Knots
	�ֶ�8����λ�ǣ���
	�ֶ�9��UTC���ڣ�DDMMYY��ʽ
	�ֶ�10����ƫ�ǣ���000 - 180���ȣ�ǰ��λ��������0��
	�ֶ�11����ƫ�Ƿ���E=��W=��
	�ֶ�16��У��ֵ
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
	����$GPVTG,89.68,T,,M,0.00,N,0.0,K*5F
	�ֶ�0��$GPVTG�����ID�����������ΪTrack Made Good and Ground Speed��VTG�������ٶ���Ϣ
	�ֶ�1���˶��Ƕȣ�000 - 359����ǰ��λ��������0��
	�ֶ�2��T=�汱����ϵ
	�ֶ�3���˶��Ƕȣ�000 - 359����ǰ��λ��������0��
	�ֶ�4��M=�ű�����ϵ
	�ֶ�5��ˮƽ�˶��ٶȣ�0.00����ǰ��λ��������0��
	�ֶ�6��N=�ڣ�Knots
	�ֶ�7��ˮƽ�˶��ٶȣ�0.00����ǰ��λ��������0��
	�ֶ�8��K=����/ʱ��km/h
	�ֶ�9��У��ֵ
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
	����$GPGLL,4250.5589,S,14718.5084,E,092204.999,A*2D
	�ֶ�0��$GPGLL�����ID�����������ΪGeographic Position��GLL������λ��Ϣ
	�ֶ�1��γ��ddmm.mmmm���ȷָ�ʽ��ǰ��λ��������0��
	�ֶ�2��γ��N����γ����S����γ��
	�ֶ�3������dddmm.mmmm���ȷָ�ʽ��ǰ��λ��������0��
	�ֶ�4������E����������W��������
	�ֶ�5��UTCʱ�䣬hhmmss.sss��ʽ
	�ֶ�6��״̬��A=��λ��V=δ��λ
	�ֶ�7��У��ֵ
*/
void gps_GPGLL(nmea_msg *gpsx, char *buf)
{

}

/*
 * NEMA-0183���ݰ�����
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
