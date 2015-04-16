#include <diag_cmd.h>
#include <bsp_os.h>
#include "stm32f4xx.h"


/*************************************************************************/

OS_SEM SEM_UART;

char console_buffer[CFG_CBSIZE];				/* console I/O buffer	*/

static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen);
static char erase_seq[] = "\b \b";			/* erase sequence	*/
static char   tab_seq[] = "        ";		/* used to expand TABs	*/


extern volatile int uart1_tstc;
/**************************************************************************
 * Post sem to readline() wehn receive char from console
 **************************************************************************/
void USART1_IRQHandler(void)
{
	OS_ERR  err;

	CPU_SR_ALLOC();
	
	CPU_CRITICAL_ENTER();  
	OSIntNestingCtr++;
	CPU_CRITICAL_EXIT();

	/* Interrupt handler */	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{ 
		USART_ClearFlag(USART1, USART_FLAG_RXNE);
		uart1_tstc = 1;
		OSSemPost (&SEM_UART,
								OS_OPT_POST_1,
								&err);
	}

	OSIntExit(); 
}


/***************************************************************************
 * check uartdata reday?
 **************************************************************************/
int tstc (void)
{
	return serial_tstc();
}

/***************************************************************************
 * test if ctrl-c was pressed
 **************************************************************************/
static int ctrlc_disabled = 0;	/* see disable_ctrl() */
static int ctrlc_was_pressed = 0;
int ctrlc (void)
{
	if (!ctrlc_disabled) {
		if(serial_tstc()) {
			switch (serial_getc()) {
			case 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				return 1;
			default:
				break;
			}
		}
	}
	return 0;
}

/***************************************************************************
 * pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 **************************************************************************/
int disable_ctrlc (int disable)
{
	int prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	return prev;
}

int had_ctrlc (void)
{
	return ctrlc_was_pressed;
}

void clear_ctrlc (void)
{
	ctrlc_was_pressed = 0;
}

/***************************************************************************
 * delete char in console
 **************************************************************************/
static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen)
{
	char *s;

	if (*np == 0) {
		return (p);
	}

	if (*(--p) == '\t') {			/* will retype the whole line	*/
		while (*colp > plen) {
			serial_puts (erase_seq);
			(*colp)--;
		}
		for (s=buffer; s<p; ++s) {
			if (*s == '\t') {
				serial_puts (tab_seq+((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} else {
				++(*colp);
				serial_putc (*s);
			}
		}
	} else {
		serial_puts (erase_seq);
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
int readline (const char *const prompt)
{
	char   *p = console_buffer;
	int	n = 0;				/* buffer index		*/
	int	plen = 0;			/* prompt length	*/
	int	col;				/* output column cnt	*/
	char	c;

	/* print prompt */
	if (prompt) {
		plen = strlen (prompt);
		serial_puts (prompt);
	}
	col = plen;

	for (;;) {
		/* wait for UART1_RX interrupt */
		OS_ERR err;
		CPU_TS ts;
		OSSemPend(&SEM_UART,
							0,
							OS_OPT_PEND_BLOCKING,
							&ts,
							&err);
			
		c = serial_getc();
		/*
		 * Special character handling
		 */
		switch (c) {
		case '\r':				/* Enter		*/
		case '\n':
			*p = '\0';
			serial_puts ("\r\n");
			return (p - console_buffer);

		case '\0':				/* nul			*/
			continue;

		case 0x03:				/* ^C - break		*/
			console_buffer[0] = '\0';	/* discard input */
			return (-1);

		case 0x15:				/* ^U - erase line	*/
			while (col > plen) {
				serial_puts (erase_seq);
				--col;
			}
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
			if (n < CFG_CBSIZE-2) {
				if (c == '\t') {	/* expand TABs		*/
#ifdef CONFIG_AUTO_COMPLETE
					/* if auto completion triggered just continue */
					*p = '\0';
					if (cmd_auto_complete(prompt, console_buffer, &n, &col)) {
						p = console_buffer + n;	/* reset */
						continue;
					}
#endif
					serial_puts (tab_seq+(col&07));
					col += 8 - (col&07);
				} else {
					++col;		/* echo input		*/
					serial_putc (c);
				}
				*p++ = c;
				++n;
			} else {			/* Buffer full		*/
				serial_putc ('\a');
			}
		}
	}
}


