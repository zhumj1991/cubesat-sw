#include "stm32f4xx.h"
#include "stdio.h"


/***********************************************************************
*	重定义printf
************************************************************************/
int fputc(int ch, FILE *f)
{
	/* 写一个字节到USART1 */
	USART_SendData(USART1, (uint8_t) ch);

	/* 等待发送结束 */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{}

	return ch;
}

/************************************************************************
*	重定义scanf
************************************************************************/
int fgetc(FILE *f)
{
	/* 等待串口1输入数据 */
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);

	return (int)USART_ReceiveData(USART1);
}

/************************************************************************
*	send a char
************************************************************************/
int serial_putc (int ch)
{
	/* 写一个字节到USART1 */
	USART_SendData(USART1, (uint8_t) ch);

	/* 等待发送结束 */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{}

	return ch;
}


/************************************************************************
 *	send strings
 ***********************************************************************/
void serial_puts (const char *s)
{
	while(*s) {
		serial_putc(*s++);
	}
}

/************************************************************************
 *receive a char from uart
 ***********************************************************************/
volatile int uart1_tstc;

int serial_getc (void)
{
	/* 等待串口1输入数据 */
//	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
	uart1_tstc = 0;
	
	return (int)USART_ReceiveData(USART1);
}

/************************************************************************
 * data received already?
* 1: received; 0: data register is empty
 ***********************************************************************/
int serial_tstc (void)
{
	if(uart1_tstc) {
		return 1;
	} else {
		return 0;
	}
	
	//return USART_GetFlagStatus(USART1, USART_FLAG_RXNE);
}
