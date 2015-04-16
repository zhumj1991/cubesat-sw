#include "includes.h"

/* RTC */
const char *weekdays[7] = {
	"Mon", "Tues", "Wednes", "Thurs", "Fri", "Satur", "Sun"
};
extern struct obc_info obc_status;
extern struct obc_tele obc_teledata;

extern uint32_t __code_start;
extern uint32_t __code_length;

extern OS_SEM   SEM_SPI1_DEV_MUTEX;

#define BUF_SIZE		16
char console_buffer[BUF_SIZE];				/* console I/O buffer	*/

static char erase_seq[] = "\b \b";			/* erase sequence	*/
static char   tab_seq[] = "        ";	


static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen)
{
	char *s;

	if (*np == 0) {
		return (p);
	}

	if (*(--p) == '\t') {			/* will retype the whole line	*/
		while (*colp > plen) {
			printf("%s", erase_seq);
			(*colp)--;
		}
		for (s=buffer; s<p; ++s) {
			if (*s == '\t') {
				printf("%s", tab_seq+((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} else {
				++(*colp);
				printf("%s", s);
			}
		}
	} else {
		printf("%s", erase_seq);
		(*colp)--;
	}
	(*np)--;
	return (p);
}

/**************************************************************************
 * Prompt for input and read a line.
 * If  CONFIG_BOOT_RETRY_TIME is defined and retry_time >= 0,
 * time out when time goes past endtime (timebase time in ticks).
 * Return:	number of read characters
 *		-1 if break
 *		-2 if timed out
 **************************************************************************/
static int readline (void)
{
	char *p = console_buffer;
	int	n = 0;				/* buffer index		*/
	int	plen = 0;			/* prompt length	*/
	int	col;				/* output column cnt	*/
	char	c;

	col = plen;

	for (;;) {
		
		while(!uart1_fifo_read((unsigned char *)&c, 1)) {
			BSP_OS_TimeDlyMs(50);
		}
		/*
		 * Special character handling
		 */
		switch (c) {
		case '\r':				/* Enter		*/
		case '\n':
			*p = '\0';
			printf("\r\n");
			return (p - console_buffer);

		case '\0':				/* nul			*/
			continue;

		case 0x03:				/* ^C - break		*/
			console_buffer[0] = '\0';	/* discard input */
			return (-1);

		case 0x15:				/* ^U - erase line	*/
			while (col > plen) {
				printf("%s", erase_seq);
				--col;
			};
			p = console_buffer;
			n = 0;
			continue;

		case 0x17:				/* ^W - erase word 	*/
			p=delete_char(console_buffer, p, &col, &n, plen);
			while ((n > 0) && (*p != ' ')) {
				p=delete_char(console_buffer, p, &col, &n, plen);
			}
			continue;

		case 0x08:				/* ^H  - backspace	*/
		case 0x7F:				/* DEL - backspace	*/
			p=delete_char(console_buffer, p, &col, &n, plen);
			continue;

		default:
			/*
			 * Must be a normal character then
			 */
			if (n < BUF_SIZE-2) {
				if (c == '\t') {	/* expand TABs		*/
					printf("%s", tab_seq+(col&07));
					col += 8 - (col&07);
				} else {
					++col;		/* echo input		*/
					printf("%c", c);
				}
				*p++ = c;
				++n;
			} else {			/* Buffer full		*/
				printf("\a");
			}
		}
	}
}


static int spi_display(char *buf, uint32_t offset, uint16_t len)
{
	#define DISP_LINE_LEN	16
	unsigned int	addr = offset;
	unsigned int	i, nbytes, linebytes;
	unsigned char	*cp;
	int rc = 0;
	
	nbytes = len;
	do {
		char	linebuf[DISP_LINE_LEN];
		uint8_t	*ucp = (uint8_t *)linebuf;

		printf("%08x:", addr);
		linebytes = (nbytes > DISP_LINE_LEN)? DISP_LINE_LEN : nbytes;

		for (i=0; i<linebytes; i++) {
			printf(" %02x", (*ucp++ = *((uint8_t *)buf)));
			buf++;
			addr++;
		}

		printf("    ");
		cp = (uint8_t *)linebuf;
		for (i=0; i<linebytes; i++) {
			if ((*cp < 0x20) || (*cp > 0x7e))
				printf(".");
			else
				printf ("%c", *cp);
			cp++;
		}
		printf("\r\n");
		nbytes -= linebytes;
	} while (nbytes > 0);
	
	return rc;
}

static void teledata_diag(void)
{
	unsigned char select = 0;
	signed char len = 0;
	unsigned int sector_addr, offset, size;
	char buf[256];
	
	OS_ERR  err;
	CPU_TS ts;

	
	while(1) {
		printf("\r\n");
		printf("## Teledata DIAG ##\r\n");	
		
		printf("[1] OBC current status\r\n");
		printf("[2] OBC current teledata\r\n");					
		printf("[3] Flash data display\r\n");
		printf("[4] Flash Erase Sector\r\n");
		printf("[5] Flash Erase Chip\r\n");
		printf("[6] Back\r\n");

		printf("Enter your Selection: ");
		
		while(!uart1_fifo_read(&select, 1)) {
			BSP_OS_TimeDlyMs(50);
		}
		
		printf("%c\r\n", select >= ' ' && select <= 127 ? select : ' ');
	
		switch(select) {				
		case '1':
			printf("CPU power on %d times\r\n", obc_status.power_on);
			printf("Date: %02d/%02d/%02d(%sday)    Time: %02d:%02d:%02d\r\n",
					obc_status.tm.tm_year, obc_status.tm.tm_mon, obc_status.tm.tm_mday,
					(obc_status.tm.tm_wday<1 || obc_status.tm.tm_wday>8) ?
					"unknown " : weekdays[obc_status.tm.tm_wday - 1],
					obc_status.tm.tm_hour, obc_status.tm.tm_min, obc_status.tm.tm_sec);
			break;	
		case '2':
			
			break;
		case '3':
			printf("Please input 'offset': ");
			len = readline();
			if (len == -1) {
				printf("<INTERRUPT>\r\n\n");
				break;
			}			
			offset = strtoul(console_buffer, NULL ,16);
			
			printf("Please input 'size': ");
			len = readline();
			if (len == -1) {
				printf("<INTERRUPT>\r\n\n");
				break;
			}	
			size = strtoul(console_buffer, NULL ,10);
			if(size > 256) {
				printf("'size' < 256\r\n\n");
				break;
			}
			
			OSSemPend(&SEM_SPI1_DEV_MUTEX, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
			spi_flash_read_buffer((unsigned char *)buf, offset, size);
			OSSemPost (&SEM_SPI1_DEV_MUTEX, OS_OPT_POST_1, &err);
			
			
			
			spi_display(buf, offset, size);
			
			break;
		case '4':
			printf("Please input 'sector_addr': ");
			len = readline();
			if (len == -1) {
				printf("<INTERRUPT>\r\n");
				break;
			}			
			sector_addr = strtoul(console_buffer, NULL ,16);
		
			spi_flash_erase_sector(sector_addr);
			break;
		/* Exit to command line */
		case '5':
			spi_flash_erase_chip();
			break;
		case '6':
			return;
		default:
			break;		
		}
	}
}

static void edac_diag(void)
{
	unsigned char select = 0;
	signed char len = 0;
	unsigned int addr;
	unsigned char bit;

	
	while(1) {
		printf("\r\n");
		printf("## EDAC DIAG ##\r\n");	
		
		printf("[1] Status\r\n");		
		printf("[2] Set one bit error\r\n");							
		printf("[3] Back\r\n");								

		printf("Enter your Selection: ");
		
		while(!uart1_fifo_read(&select, 1)) {
			BSP_OS_TimeDlyMs(50);
		}
		
		printf("%c\r\n", select >= ' ' && select <= 127 ? select : ' ');
	
		switch(select) {				
		case '1':
			printf("Num of edac errors: %d\r\n", obc_teledata.edac_errors);
			break;	
		case '2':
			printf("Please input the address[0x%8x~0x%8x]: ", __code_start, (__code_start + __code_length));

			len = readline();
			if (len == -1) {
				printf("<INTERRUPT>\r\n");
				break;
			}
			
			addr = strtoul(console_buffer, NULL ,16);
			if(addr < __code_start || addr > (__code_start + __code_length)) {
				printf("Wrong address\r\n");
				break;
			}	
			printf("The origin value of 0x%08x : 0x%02x\r\n\n", addr, *(unsigned char *)addr);

			printf("Please input the bit[0~7] you want to upset: ");
			
			len = readline();
			if (len == -1) {
				printf("<INTERRUPT>\r\n");
				break;
			}
			
			bit = strtoul(console_buffer, NULL ,16);
			if(bit > 7) {
				printf("Wrong bit(bit: 0~7)\r\n");
				break;
			}
						
			*(unsigned char *)addr ^= (1 << bit);
			printf("The value of 0x%08x : 0x%02x\r\n", addr,  *(unsigned char *)addr);
			
			break;
		/* Exit to command line */
		case '3':
			return;
		default:
			break;		
		}
	}
}



static void diag_menu(void)
{
	unsigned char select;
	struct rtc_time tm;
	
	while(1) {
		printf("\r\n");
		printf("############# User Menu for STM32 #############\r\n");	
		
		printf("[1] RTC Time Show\r\n");		
		printf("[2] Teledata\r\n");							
		printf("[3] EDAC\r\n");
		printf("[4] Reset\r\n");
		printf("[5] Exit\r\n");								

		printf("---------------------Select--------------------\r\n");
		printf("Enter your Selection: ");
		
		while(!uart1_fifo_read(&select, 1)) {
			BSP_OS_TimeDlyMs(50);
		}
		
		printf("%c\r\n", select >= ' ' && select <= 127 ? select : ' ');
	
		switch(select) {					
		case '1':
			rtc_get(&tm);
			debug("Date: %02d/%02d/%02d(%sday)    Time: %02d:%02d:%02d\r\n",
				tm.tm_year, tm.tm_mon, tm.tm_mday,
				(tm.tm_wday<1 || tm.tm_wday>8) ?
				"unknown " : weekdays[tm.tm_wday - 1],
				tm.tm_hour, tm.tm_min, tm.tm_sec);
			break;	
		case '2':
			teledata_diag();
			break;
		case '3':
			edac_diag();
			break;	
		case '4':
			printf("reset...\r\n\n\n");
			system_reset();
			break;
		case '5':
			return;
		default:
			break;		
		}
			
	}
}

void diag(void)
{
	diag_menu();
	
	while(1){

		BSP_OS_TimeDlyMs(50);
	}	
}
