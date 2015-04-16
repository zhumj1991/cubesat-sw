/*
*********************************************************************************************************
*
*	ģ������ : SPI�ӿڴ���FLASH ��дģ��
*	�ļ����� : bsp_spi_flash.c
*	��    �� : V1.0
*	˵    �� : ֧�� SST25VF016B��MX25L1606E �� W25Q64BVSSIG
*			   		 SST25VF016B ��д��������AAIָ��������д��Ч�ʡ�
*********************************************************************************************************
*/
#include "includes.h"

/*
	STM32F4XX ʱ�Ӽ���.
		HCLK = 168M
		PCLK1 = HCLK / 4 = 42M
		PCLK2 = HCLK / 2 = 84M

		SPI2��SPI3 �� PCLK1, ʱ��42M
		SPI1       �� PCLK2, ʱ��84M

		STM32F4 ֧�ֵ����SPIʱ��Ϊ 37.5 Mbits/S, �����Ҫ��Ƶ��
*/


#if	SW_ON_ARMFLY
	#define SPI_FLASH			SPI1
	#define SPI_BAUD			SPI_BaudRatePrescaler_4
	#define SF_CS_GPIO		GPIOF
	#define SF_CS_PIN			GPIO_Pin_8
#else
	#define SPI_FLASH			SPI1
	#define SPI_BAUD			SPI_BaudRatePrescaler_4
	#define SF_CS_GPIO		GPIOC
	#define SF_CS_PIN			GPIO_Pin_4
#endif

#define SF_CS_LOW()       SF_CS_GPIO->BSRRH = SF_CS_PIN
#define SF_CS_HIGH()      SF_CS_GPIO->BSRRL = SF_CS_PIN


spi_flash_info flash_info;


void spi_flash_read_info(void);
static uint8_t spi_flash_send_byte(uint8_t value);
static void spi_flash_write_enable(void);
static void spi_flash_write_status(uint8_t value);
static uint8_t spi_flash_wait_ready(void);
static uint8_t spi_flash_need_erase(uint8_t *old_buf, uint8_t *new_buf, uint16_t len);
static uint8_t spi_flash_auto_write_sector(uint8_t *buf, uint32_t write_addr, uint16_t size);
static uint8_t spi_flash_cmp_data(uint32_t src_addr, uint8_t *tar, uint32_t size);


static uint8_t spi_buf[4 * 1024];	/* ����д�������ȶ�������sector���޸Ļ�������������page��д */


/*
*********************************************************************************************************
*	�� �� ��: bsp_InitSpiFlash
*	����˵��: ��ʼ������FlashӲ���ӿڣ�����STM32��SPIʱ�ӡ�GPIO)
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint8_t bsp_spi_flash_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	uint8_t ret;

	/* COnfigure GPIOs */
#if	SW_ON_ARMFLY
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOF, ENABLE);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
#else
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
	GPIO_InitStructure.GPIO_Pin = SF_CS_PIN;
	GPIO_Init(SF_CS_GPIO, &GPIO_InitStructure);

	SF_CS_HIGH();											/* Ƭѡ�øߣ���ѡ�� */

	/* ����SPIӲ���������ڷ��ʴ���Flash */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	SPI_Cmd(SPI_FLASH, DISABLE);			/* ��ֹSPI  */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* ���ݷ���2��ȫ˫�� */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;												/* STM32��SPI����ģʽ ������ģʽ */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;										/* ����λ���� �� 8λ */
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;													/* ʱ�������ز������� */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;												/* ʱ�ӵĵ�2�����ز������� */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;														/* Ƭѡ���Ʒ�ʽ��������� */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BAUD;									/* ���ò�����Ԥ��Ƶϵ�� */
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;									/* ����λ������򣺸�λ�ȴ� */
	SPI_InitStructure.SPI_CRCPolynomial = 7;														/* CRC����ʽ�Ĵ�������λ��Ϊ7�������̲��� */
	SPI_Init(SPI_FLASH, &SPI_InitStructure);
	SPI_Cmd(SPI_FLASH, ENABLE);			/* ʹ��SPI  */

	spi_flash_read_info();					/* �Զ�ʶ��оƬ�ͺ� */

	SF_CS_LOW();										/* �����ʽ��ʹ�ܴ���FlashƬѡ */
	spi_flash_send_byte(CMD_DISWR);	/* ���ͽ�ֹд�������,��ʹ�����д���� */
	SF_CS_HIGH();										/* �����ʽ�����ܴ���FlashƬѡ */

	ret = spi_flash_wait_ready();					/* �ȴ�����Flash�ڲ�������� */
	spi_flash_write_status(0);			/* �������BLOCK��д���� */
	
	return ret;
}

/*
*********************************************************************************************************
*	�� �� ��: spi_flash_read_chipID
*	����˵��: ��ȡ����ID
*	��    ��: ��
*	�� �� ֵ: 32bit������ID (���8bit��0����ЧIDλ��Ϊ24bit��
*********************************************************************************************************
*/
uint32_t spi_flash_read_chipID(void)
{
	uint32_t ID;
	uint8_t id1, id2, id3;

	SF_CS_LOW();									/* ʹ��Ƭѡ */
	spi_flash_send_byte(CMD_RDID);									/* ���Ͷ�ID���� */
	id1 = spi_flash_send_byte(DUMMY_BYTE);					/* ��ID�ĵ�1���ֽ� */
	id2 = spi_flash_send_byte(DUMMY_BYTE);					/* ��ID�ĵ�2���ֽ� */
	id3 = spi_flash_send_byte(DUMMY_BYTE);					/* ��ID�ĵ�3���ֽ� */
	SF_CS_HIGH();									/* ����Ƭѡ */

	ID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;

	return ID;
}

/*
*********************************************************************************************************
*	�� �� ��: spi_flash_read_info
*	����˵��: ��ȡ����ID,�������������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void spi_flash_read_info(void)
{
	/* �Զ�ʶ����Flash�ͺ� */
	{
		flash_info.chipID = spi_flash_read_chipID();		/* оƬID */

		switch (flash_info.chipID)
		{
			case SST25VF016B_ID:
				strcpy(flash_info.chip_name, "SST25VF016B");
				flash_info.total_size = 2 * 1024 * 1024;/* ������ = 2M */
				flash_info.sector_size = 4 * 1024;				/* ҳ���С = 4K */
				break;

			case MX25L1606E_ID:
				strcpy(flash_info.chip_name, "MX25L1606E");
				flash_info.total_size = 2 * 1024 * 1024;/* ������ = 2M */
				flash_info.sector_size = 4 * 1024;				/* ҳ���С = 4K */
				break;

			case W25Q64BV_ID:
				strcpy(flash_info.chip_name, "W25Q64BV");
				flash_info.total_size = 8 * 1024 * 1024;/* ������ = 8M */
				flash_info.sector_size = 4 * 1024;
				break;
			
			case W25Q128FV_ID:
				strcpy(flash_info.chip_name, "W25Q128FV");
				flash_info.total_size = 16 * 1024 * 1024;/* ������ = 16M */
				flash_info.sector_size = 4 * 1024;
				break;

			default:
				strcpy(flash_info.chip_name, "Unknow Flash");
				flash_info.total_size = 2 * 1024 * 1024;
				flash_info.sector_size = 4 * 1024;
				break;
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: spi_flash_erase_sector
*	����˵��: ����ָ��������
*	��    ��: sector_addr : ������ַ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void spi_flash_erase_sector(uint32_t sector_addr)
{
	spi_flash_write_enable();			/* ����дʹ������ */

	/* ������������ */
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	spi_flash_send_byte(CMD_SE);													/* ���Ͳ������� */
	spi_flash_send_byte((sector_addr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
	spi_flash_send_byte((sector_addr & 0xFF00) >> 8);			/* ����������ַ�м�8bit */
	spi_flash_send_byte(sector_addr & 0xFF);							/* ����������ַ��8bit */
	SF_CS_HIGH();									/* ����Ƭѡ */

	spi_flash_wait_ready();							/* �ȴ�����Flash�ڲ�д������� */
}

/*
*********************************************************************************************************
*	�� �� ��: spi_flash_erase_chip
*	����˵��: ��������оƬ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void spi_flash_erase_chip(void)
{
	spi_flash_write_enable();			/* ����дʹ������ */

	/* ������������ */
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	spi_flash_send_byte(CMD_BE);		/* ������Ƭ�������� */
	SF_CS_HIGH();									/* ����Ƭѡ */

	spi_flash_wait_ready();	/* �ȴ�����Flash�ڲ�д������� */
}

/*
*********************************************************************************************************
*	�� �� ��: spi_flash_write_enable
*	����˵��: ����������дʹ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void spi_flash_write_enable(void)
{
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	spi_flash_send_byte(CMD_WREN);	/* �������� */
	SF_CS_HIGH();									/* ����Ƭѡ */
}

/*
*********************************************************************************************************
*	�� �� ��: spi_flash_write_status
*	����˵��: д״̬�Ĵ���
*	��    ��: value : ״̬�Ĵ�����ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void spi_flash_write_status(uint8_t value)
{

	if (flash_info.chipID == SST25VF016B_ID)
	{
		/* ��1������ʹ��д״̬�Ĵ��� */
		SF_CS_LOW();									/* ʹ��Ƭѡ */
		spi_flash_send_byte(CMD_EWRSR);/* ������� ����д״̬�Ĵ��� */
		SF_CS_HIGH();									/* ����Ƭѡ */

		/* ��2������д״̬�Ĵ��� */
		SF_CS_LOW();									/* ʹ��Ƭѡ */
		spi_flash_send_byte(CMD_WRSR);	/* ������� д״̬�Ĵ��� */
		spi_flash_send_byte(value);		/* �������ݣ�״̬�Ĵ�����ֵ */
		SF_CS_HIGH();									/* ����Ƭѡ */
	}
	else
	{
		SF_CS_LOW();									/* ʹ��Ƭѡ */
		spi_flash_send_byte(CMD_WRSR);	/* ������� д״̬�Ĵ��� */
		spi_flash_send_byte(value);		/* �������ݣ�״̬�Ĵ�����ֵ */
		SF_CS_HIGH();									/* ����Ƭѡ */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: spi_flash_wait_ready
*	����˵��: ����ѭ����ѯ�ķ�ʽ�ȴ������ڲ�д�������
*	��    ��:  ��
*	�� �� ֵ: 0 �ɹ��� 1 ��ʱ
*********************************************************************************************************
*/
static uint8_t spi_flash_wait_ready(void)
{
	uint32_t timeout = 100000; 
	
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	spi_flash_send_byte(CMD_RDSR);/* ������� ��״̬�Ĵ��� */
	while(((spi_flash_send_byte(DUMMY_BYTE) & WIP_FLAG) == SET) && (timeout--))	{ /* �ж�״̬�Ĵ�����æ��־λ */
		BSP_DelayUS(10);
	}
	SF_CS_HIGH();									/* ����Ƭѡ */
	
	if(timeout)
		return 0;
	else
		return 1;
}

/*
*********************************************************************************************************
*	�� �� ��: spi_flash_cmp_data
*	����˵��: �Ƚ�Flash������.
*	��    ��: tar 			: ���ݻ�����
*						src_addr	��Flash��ַ
*						size 			�����ݸ���, ���Դ���PAGE_SIZE,���ǲ��ܳ���оƬ������
*	�� �� ֵ: 0 = ���, 1 = ����
*********************************************************************************************************
*/
static uint8_t spi_flash_cmp_data(uint32_t src_addr, uint8_t *tar, uint32_t size)
{
	uint8_t value;

	/* �����ȡ�����ݳ���Ϊ0���߳�������Flash��ַ�ռ䣬��ֱ�ӷ��� */
	if ((src_addr + size) > flash_info.total_size)
	{
		return 1;
	}

	if (size == 0)
	{
		return 0;
	}

	SF_CS_LOW();									/* ʹ��Ƭѡ */
	spi_flash_send_byte(CMD_READ);							/* ���Ͷ����� */
	spi_flash_send_byte((src_addr & 0xFF0000) >> 16);		/* ����������ַ�ĸ�8bit */
	spi_flash_send_byte((src_addr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
	spi_flash_send_byte(src_addr & 0xFF);					/* ����������ַ��8bit */
	while (size--)
	{
		/* ��һ���ֽ� */
		value = spi_flash_send_byte(DUMMY_BYTE);
		if (*tar++ != value)
		{
			SF_CS_HIGH();
			return 1;
		}
	}
	SF_CS_HIGH();
	return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: spi_flash_need_erase
*	����˵��: �ж�дPAGEǰ�Ƿ���Ҫ�Ȳ�����
*	��    ��: old_buf ��������
*						new_buf ��������
*						len 		�����ݸ��������ܳ���ҳ���С
*	�� �� ֵ: 0 : ����Ҫ������ 1 ����Ҫ����
*********************************************************************************************************
*/
static uint8_t spi_flash_need_erase(uint8_t *old_buf, uint8_t *new_buf, uint16_t len)
{
	uint16_t i;
	uint8_t old;

	/*
	�㷨��1����old ��, new ����
	    old    new
		  1101   0101
	~   1111
		= 0010   0101

	�㷨��2��: old �󷴵Ľ���� new λ��
		  0010   old
	&	  0101   new
		 =0000

	�㷨��3��: ���Ϊ0,���ʾ�������. �����ʾ��Ҫ����
	*/

	for (i = 0; i < len; i++)
	{
		old = *old_buf++;
		old = ~old;

		/* ע������д��: if (ucOld & (*new_buf++) != 0) */
		if ((old & (*new_buf++)) != 0)
		{
			return 1;
		}
	}
	return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: spi_flash_auto_write_sector
*	����˵��: д1��PAGE��У��,�������ȷ������д���Ρ��������Զ���ɲ���������
*	��    ��: buf 				: ����Դ��������
*						write_addr	��Ŀ�������׵�ַ
*						size 				�����ݸ��������ܳ���ҳ���С
*	�� �� ֵ: 0 : ���� 1 �� �ɹ�
*********************************************************************************************************
*/
static uint8_t spi_flash_auto_write_sector(uint8_t *buf, uint32_t write_addr, uint16_t size)
{
	uint16_t i;
	uint16_t j;					/* ������ʱ */
	uint32_t first_addr;	/* ������ַ */
	uint8_t need_erase;		/* 1��ʾ��Ҫ���� */
	uint8_t ret;

	/* ����Ϊ0ʱ����������,ֱ����Ϊ�ɹ� */
	if (size == 0)
	{
		return 1;
	}

	/* ���ƫ�Ƶ�ַ����оƬ�������˳� */
	if (write_addr >= flash_info.total_size)
	{
		return 0;
	}

	/* ������ݳ��ȴ����������������˳� */
	if (size > flash_info.sector_size)
	{
		return 0;
	}

	/* ���FLASH�е�����û�б仯,��дFLASH */
	spi_flash_read_buffer(spi_buf, write_addr, size);
	if (memcmp(spi_buf, buf, size) == 0)
	{
		return 1;
	}

	/* �ж��Ƿ���Ҫ�Ȳ������� */
	/* ����������޸�Ϊ�����ݣ�����λ���� 1->0 ���� 0->0, ���������,���Flash���� */
	need_erase = 0;
	if (spi_flash_need_erase(spi_buf, buf, size))
	{
		need_erase = 1;
	}

	first_addr = write_addr & (~(flash_info.sector_size - 1));

	if (size == flash_info.sector_size)		/* ������������д */
	{
		for	(i = 0; i < flash_info.sector_size; i++)
		{
			spi_buf[i] = buf[i];
		}
	}
	else						/* ��д�������� */
	{
		/* �Ƚ��������������ݶ��� */
		spi_flash_read_buffer(spi_buf, first_addr, flash_info.sector_size);

		/* ���������ݸ��� */
		i = write_addr & (flash_info.sector_size - 1);
		memcpy(&spi_buf[i], buf, size);
	}

	/* д��֮�����У�飬�������ȷ����д�����3�� */
	ret = 0;
	for (i = 0; i < 3; i++)
	{

		/* ����������޸�Ϊ�����ݣ�����λ���� 1->0 ���� 0->0, ���������,���Flash���� */
		if (need_erase == 1)
		{
			spi_flash_erase_sector(first_addr);		/* ����1������ */
		}

		/* ���һ��PAGE */
		spi_flash_write_sector(spi_buf, first_addr, flash_info.sector_size);

		if (spi_flash_cmp_data(write_addr, buf, size) == 0)
		{
			ret = 1;
			break;
		}
		else
		{
			if (spi_flash_cmp_data(write_addr, buf, size) == 0)
			{
				ret = 1;
				break;
			}

			/* ʧ�ܺ��ӳ�һ��ʱ�������� */
			for (j = 0; j < 10000; j++);
		}
	}

	return ret;
}

/*
*********************************************************************************************************
*	�� �� ��: spi_flash_write_sector
*	����˵��: ��һ��sector��д�������ֽڡ��ֽڸ������ܳ���ҳ���С��4K)
*	��    ��: buf 			: ����Դ��������
*						WriteAddr ��Ŀ�������׵�ַ
*						Size 			�����ݸ��������ܳ���ҳ���С
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void spi_flash_write_sector(uint8_t *buf, uint32_t write_addr, uint16_t size)
{
	uint32_t i, j;

	if (flash_info.chipID == SST25VF016B_ID)
	{
		/* AAIָ��Ҫ��������ݸ�����ż�� */
		if ((size<2) && (size%2))
		{
			return ;
		}

		spi_flash_write_enable();			/* ����дʹ������ */

		SF_CS_LOW();									/* ʹ��Ƭѡ */
		spi_flash_send_byte(CMD_AAI);												/* ����AAI����(��ַ�Զ����ӱ��) */
		spi_flash_send_byte((write_addr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
		spi_flash_send_byte((write_addr & 0xFF00) >> 8);			/* ����������ַ�м�8bit */
		spi_flash_send_byte(write_addr & 0xFF);							/* ����������ַ��8bit */
		spi_flash_send_byte(*buf++);													/* ���͵�1������ */
		spi_flash_send_byte(*buf++);													/* ���͵�2������ */
		SF_CS_HIGH();									/* ����Ƭѡ */

		spi_flash_wait_ready();	/* �ȴ�����Flash�ڲ�д������� */

		size -= 2;										/* ����ʣ���ֽ��� */

		for (i = 0; i<(size/2); i++)
		{
			SF_CS_LOW();								/* ʹ��Ƭѡ */
			spi_flash_send_byte(CMD_AAI);											/* ����AAI����(��ַ�Զ����ӱ��) */
			spi_flash_send_byte(*buf++);												/* �������� */
			spi_flash_send_byte(*buf++);												/* �������� */
			SF_CS_HIGH();								/* ����Ƭѡ */
			spi_flash_wait_ready();/* �ȴ�����Flash�ڲ�д������� */
		}

		/* ����д����״̬ */
		SF_CS_LOW();
		spi_flash_send_byte(CMD_DISWR);
		SF_CS_HIGH();

		spi_flash_wait_ready();	/* �ȴ�����Flash�ڲ�д������� */
	}
	else	/* for MX25L1606E �� W25Q64BV */
	{
		for (j=0; j<(size/256); j++)
		{
			spi_flash_write_enable();								/* ����дʹ������ */

			SF_CS_LOW();									/* ʹ��Ƭѡ */
			spi_flash_send_byte(0x02);														/* ����AAI����(��ַ�Զ����ӱ��) */
			spi_flash_send_byte((write_addr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
			spi_flash_send_byte((write_addr & 0xFF00) >> 8);			/* ����������ַ�м�8bit */
			spi_flash_send_byte(write_addr & 0xFF);							/* ����������ַ��8bit */

			for (i=0; i<256; i++)
			{
				spi_flash_send_byte(*buf++);												/* �������� */
			}

			SF_CS_HIGH();								/* ��ֹƬѡ */

			spi_flash_wait_ready();/* �ȴ�����Flash�ڲ�д������� */

			write_addr += 256;
		}

		/* ����д����״̬ */
		SF_CS_LOW();
		spi_flash_send_byte(CMD_DISWR);
		SF_CS_HIGH();

		spi_flash_wait_ready();	/* �ȴ�����Flash�ڲ�д������� */
	}
}

void spi_flash_read_sector(uint8_t *buf, uint32_t read_addr, uint16_t size)
{
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	spi_flash_send_byte(CMD_READ);												/* ���Ͷ����� */
	spi_flash_send_byte((read_addr & 0xFF0000) >> 16);		/* ����������ַ�ĸ�8bit */
	spi_flash_send_byte((read_addr & 0xFF00) >> 8);				/* ����������ַ�м�8bit */
	spi_flash_send_byte(read_addr & 0xFF);								/* ����������ַ��8bit */
	while (size--)
	{
		*buf++ = spi_flash_send_byte(DUMMY_BYTE);					/* ��һ���ֽڲ��洢��pBuf�������ָ���Լ�1 */
	}
	SF_CS_HIGH();									/* ����Ƭѡ */
}

/*
*********************************************************************************************************
*	�� �� ��: spi_flash_write_buffer
*	����˵��: д1��������У��,�������ȷ������д���Ρ��������Զ���ɲ���������
*	��    ��: buf 				: ����Դ��������
*						write_addr	��Ŀ�������׵�ַ
*						size 				�����ݸ��������ܳ���ҳ���С
*	�� �� ֵ: 1 : �ɹ��� 0 �� ʧ��
*********************************************************************************************************
*/
uint8_t spi_flash_write_buffer(uint8_t *buf, uint32_t write_addr, uint16_t size)
{
	uint16_t NumOfSector = 0, NumOfSingle = 0, add = 0, count = 0, temp = 0;

	add = write_addr % flash_info.sector_size;
	count = flash_info.sector_size - add;
	NumOfSector =  size / flash_info.sector_size;
	NumOfSingle = size % flash_info.sector_size;

	if (add == 0) /* ��ʼ��ַ��ҳ���׵�ַ  */
	{
		if (NumOfSector == 0) /* ���ݳ���С��ҳ���С */
		{
			if (spi_flash_auto_write_sector(buf, write_addr, size) == 0)
			{
				return 0;
			}
		}
		else 	/* ���ݳ��ȴ��ڵ���ҳ���С */
		{
			while (NumOfSector--)
			{
				if (spi_flash_auto_write_sector(buf, write_addr, flash_info.sector_size) == 0)
				{
					return 0;
				}
				write_addr +=  flash_info.sector_size;
				buf += flash_info.sector_size;
			}
			if (spi_flash_auto_write_sector(buf, write_addr, NumOfSingle) == 0)
			{
				return 0;
			}
		}
	}
	else  /* ��ʼ��ַ����ҳ���׵�ַ  */
	{
		if (NumOfSector == 0) /* ���ݳ���С��ҳ���С */
		{
			if (NumOfSingle > count) /* (_usWriteSize + _uiWriteAddr) > SPI_FLASH_PAGESIZE */
			{
				temp = NumOfSingle - count;

				if (spi_flash_auto_write_sector(buf, write_addr, count) == 0)
				{
					return 0;
				}

				write_addr +=  count;
				buf += count;

				if (spi_flash_auto_write_sector(buf, write_addr, temp) == 0)
				{
					return 0;
				}
			}
			else
			{
				if (spi_flash_auto_write_sector(buf, write_addr, size) == 0)
				{
					return 0;
				}
			}
		}
		else	/* ���ݳ��ȴ��ڵ���ҳ���С */
		{
			size -= count;
			NumOfSector =  size / flash_info.sector_size;
			NumOfSingle = size % flash_info.sector_size;

			if (spi_flash_auto_write_sector(buf, write_addr, count) == 0)
			{
				return 0;
			}

			write_addr +=  count;
			buf += count;

			while (NumOfSector--)
			{
				if (spi_flash_auto_write_sector(buf, write_addr, flash_info.sector_size) == 0)
				{
					return 0;
				}
				write_addr +=  flash_info.sector_size;
				buf += flash_info.sector_size;
			}

			if (NumOfSingle != 0)
			{
				if (spi_flash_auto_write_sector(buf, write_addr, NumOfSingle) == 0)
				{
					return 0;
				}
			}
		}
	}
	
	return 1;	/* �ɹ� */
}

/*
*********************************************************************************************************
*	�� �� ��: spi_flash_read_buffer
*	����˵��: ������ȡ�����ֽڡ��ֽڸ������ܳ���оƬ������
*	��    ��: buf 			: ����Դ��������
*						read_addr ���׵�ַ
*						size 			�����ݸ���, ���Դ���PAGE_SIZE,���ǲ��ܳ���оƬ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void spi_flash_read_buffer(uint8_t *buf, uint32_t read_addr, uint32_t size)
{
	/* �����ȡ�����ݳ���Ϊ0���߳�������Flash��ַ�ռ䣬��ֱ�ӷ��� */
	if ((size == 0) ||(read_addr + size) > flash_info.total_size)
	{
		return;
	}

	SF_CS_LOW();									/* ʹ��Ƭѡ */
	spi_flash_send_byte(CMD_READ);												/* ���Ͷ����� */
	spi_flash_send_byte((read_addr & 0xFF0000) >> 16);		/* ����������ַ�ĸ�8bit */
	spi_flash_send_byte((read_addr & 0xFF00) >> 8);			/* ����������ַ�м�8bit */
	spi_flash_send_byte(read_addr & 0xFF);								/* ����������ַ��8bit */
	while (size--)
	{
		*buf++ = spi_flash_send_byte(DUMMY_BYTE);					/* ��һ���ֽڲ��洢��pBuf�������ָ���Լ�1 */
	}
	SF_CS_HIGH();									/* ����Ƭѡ */
}


/*
*********************************************************************************************************
*	�� �� ��: spi_flash_send_byte
*	����˵��: ����������һ���ֽڣ�ͬʱ��MISO���߲����������ص�����
*	��    ��: value : ���͵��ֽ�ֵ
*	�� �� ֵ: ��MISO���߲����������ص�����
*********************************************************************************************************
*/
static uint8_t spi_flash_send_byte(uint8_t value)
{
	/* �ȴ��ϸ�����δ������� */
	while (SPI_I2S_GetFlagStatus(SPI_FLASH, SPI_I2S_FLAG_TXE) == RESET);

	/* ͨ��SPIӲ������1���ֽ� */
	SPI_I2S_SendData(SPI_FLASH, value);

	/* �ȴ�����һ���ֽ�������� */
	while (SPI_I2S_GetFlagStatus(SPI_FLASH, SPI_I2S_FLAG_RXNE) == RESET);

	/* ���ش�SPI���߶��������� */
	return SPI_I2S_ReceiveData(SPI_FLASH);
}
