#include "bsp.h"


/* AD7490 Control Register */
#define RES_MASK(x)						((1 << (x)) - 1)
#define AD7490_CTRL_MASK			RES_MASK(12)

#define WRITE									(1 << 11)

#define	SEQ_NORMAL						0
#define SEQ_PROGRAM						(1 << 3)


#define ADD_CHANNEL(x)				(x << 6)

#define POWER_NORMAL					(3 << 4)
#define	POWER_FULL_SHUTDOWN		(2 << 4)
#define	POWER_AUTO_SHUTDOWN		(1 << 4)
#define POWER_AUTO_STANDBY		(0 << 4)

#define	WEAK									(1 << 2)

#define RANGE_2V5							(0 << 1)
#define RANGE_5V							(1 << 1)

#define	DATA_BIN							1
#define DATA_TWOS							0

#define	DUMMY									0xFFFF
#define AD7490_READ						0x0000



#define AD7490_CS_GPIO			GPIOA
#define AD7490_CS_PIN				GPIO_Pin_11

#define AD7490_CS_HIGH()		AD7490_CS_GPIO->BSRRL = AD7490_CS_PIN
#define AD7490_CS_LOW()			AD7490_CS_GPIO->BSRRH = AD7490_CS_PIN

static uint16_t spi_send(uint16_t value);

void bsp_ad7490_init(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	SPI_InitTypeDef  	SPI_InitStructure;
	
	/* SPI2 GPIO */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI1);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* AD7490 CS# */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = AD7490_CS_PIN;
	GPIO_Init(AD7490_CS_GPIO, &GPIO_InitStructure);
	
	/* ����SPIӲ���������ڷ��ʴ���Flash */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	SPI_Cmd(SPI2, DISABLE);			/* ��ֹSPI  */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* ���ݷ���2��ȫ˫�� */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;												/* STM32��SPI����ģʽ ������ģʽ */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;									/* ����λ���� */
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;													/* ʱ�������ز������� */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;												/* ʱ�ӵĵ�2�����ز������� */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;														/* Ƭѡ���Ʒ�ʽ��������� */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;	/* ���ò�����Ԥ��Ƶϵ�� */
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;									/* ����λ������򣺸�λ�ȴ� */
	SPI_InitStructure.SPI_CRCPolynomial = 7;														/* CRC����ʽ�Ĵ�������λ��Ϊ7�������̲��� */
	SPI_Init(SPI2, &SPI_InitStructure);
	SPI_Cmd(SPI2, ENABLE);			/* ʹ��SPI  */
	
	AD7490_CS_LOW();
}

void ad7490_reset(void)
{
	spi_send(DUMMY);
}

uint16_t ad7490_normal_convert(uint8_t channel)
{
	uint16_t command = 0;
	
	command = WRITE | SEQ_NORMAL | ADD_CHANNEL(channel) | \
						POWER_NORMAL | RANGE_5V | DATA_BIN;
	
	return spi_send(command << 4);
}

void ad7490_program_convert(uint16_t channel_en)
{
	uint16_t command = 0;
	
	command = WRITE | SEQ_PROGRAM | ADD_CHANNEL(0) | \
						SEQ_PROGRAM | RANGE_5V | DATA_BIN;
	
	spi_send(command << 4);
	
	spi_send(channel_en);
}

void ad7490_read(uint16_t *value)
{
	*value = spi_send(AD7490_READ);
}

static uint16_t spi_send(uint16_t value)
{
	AD7490_CS_LOW();

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	SPI_I2S_SendData(SPI2, value);

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

	AD7490_CS_HIGH();

	return SPI_I2S_ReceiveData(SPI2);
}
