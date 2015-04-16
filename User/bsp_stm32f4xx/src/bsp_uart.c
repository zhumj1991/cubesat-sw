#include "includes.h"


static void diag_uart_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#if	SW_ON_ARMFLY
	/* ����1 TX = PA9   RX = PA10 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	/* ���� USART Tx Ϊ���ù��� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	/* �������Ϊ���� */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* �ڲ���������ʹ�� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	/* ����ģʽ */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* ���� USART Rx Ϊ���ù��� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#else
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

	/* ���� USART Tx Ϊ���ù��� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	/* �������Ϊ���� */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* �ڲ���������ʹ�� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	/* ����ģʽ */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* ���� USART Rx Ϊ���ù��� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif

	/* ��2���� ���ô���Ӳ������ */
	USART_InitStructure.USART_BaudRate = 115200;	/* ������ */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	/* ʹ��USART1�ж� */
	USART_Cmd(USART1, ENABLE);		/* ʹ�ܴ��� */

	/* CPU��Сȱ�ݣ��������úã����ֱ��Send�����1���ֽڷ��Ͳ���ȥ
		�����������1���ֽ��޷���ȷ���ͳ�ȥ������ */
	USART_ClearFlag(USART1, USART_FLAG_TC);     /* �巢����ɱ�־��Transmission Complete flag */
}

static void gps_uart_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	/* ���� GPIO */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART1);

	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	/* �������Ϊ���� */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* �ڲ���������ʹ�� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	/* ����ģʽ */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* ���ô���Ӳ������ */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	USART_InitStructure.USART_BaudRate = 115200;	/* ������ */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	/* ʹ��USART1�ж� */
	USART_Cmd(USART2, ENABLE);		/* ʹ�ܴ��� */

	USART_ClearFlag(USART2, USART_FLAG_TC);     /* �巢����ɱ�־��Transmission Complete flag */
}

static void nvic_init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Enable the USARTx Interrupt */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

#if CFG_UART_GPS
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
}
/*
*********************************************************************************************************
*	�� �� ��: bsp_uart_init
*	����˵��: ��ʼ��CPU��USART1����Ӳ���豸��δ�����жϡ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_uart_init(void)
{
	diag_uart_init();
	
#if CFG_UART_GPS
	gps_uart_init();
#endif
	
	nvic_init();
}

/***********************************************************************
*	�ض���printf
************************************************************************/
int fputc(int ch, FILE *f)
{
	/* дһ���ֽڵ�USART1 */
	USART_SendData(USART1, (uint8_t) ch);

	/* �ȴ����ͽ��� */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{}

	return ch;
}

/************************************************************************
*	�ض���scanf
************************************************************************/
int fgetc(FILE *f)
{
	/* �ȴ�����1�������� */
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);

	return (int)USART_ReceiveData(USART1);
}

/************************************************************************
*	send a char
************************************************************************/
uint8_t serial_putc (uint8_t ch)
{
	/* дһ���ֽڵ�USART1 */
	USART_SendData(USART1, (uint8_t) ch);

	/* �ȴ����ͽ��� */
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

uint8_t serial_getc (void)
{
	/* �ȴ�����1�������� */
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
	
	return (uint8_t)USART_ReceiveData(USART1);
}

/************************************************************************
 * uart1 FIFO
 ***********************************************************************/
#define	UART1_FIFO_SIZE		256
static unsigned char uart_fifo[UART1_FIFO_SIZE];
static volatile unsigned char r = 0, w = 0;

static int isEmpty(void)
{
	return (r == w);
}

static int isFull(void)
{
	return (r == ((w+1)%UART1_FIFO_SIZE));
}

static int putData(char data)
{
	if (isFull())
		return 0;
	else
	{
		uart_fifo[w] = data;
		w = (w+1) % UART1_FIFO_SIZE;
		return 1;
	}
}

static int getData(unsigned char *p)
{
	if (isEmpty())
	{
		return 0;
	}
	else
	{
		*p = uart_fifo[r];
		r = (r+1) % UART1_FIFO_SIZE;
		return 1;
	}
}

int uart1_fifo_write(unsigned char *data, int size)
{
	int len = 0;
	unsigned char * p = data;
	
	while(size-- > 0) {
		if (putData(*p++)) {
			len++;
		} else {
			return len;
		}
	}
	
	return len;
}

int uart1_fifo_read(unsigned char *data, int size)
{
	int len = 0;
	unsigned char * p = data;
	
		while(size-- > 0) {
		if (getData(p++)) {
			len++;
		} else {
			return len;
		}
	}
	
	return len;

}

void USART1_IRQHandler(void)
{
	CPU_SR_ALLOC();
	
	CPU_CRITICAL_ENTER();  
	OSIntNestingCtr++;
	CPU_CRITICAL_EXIT();

	/* Interrupt handler */	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{ 
		USART_ClearFlag(USART1, USART_FLAG_RXNE);
		putData((uint8_t)USART_ReceiveData(USART1));
	}

	OSIntExit(); 
}
