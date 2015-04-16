#include	"includes.h"

/**
  * @brief  Configures the CAN, transmit and receive by polling
  * @param  None
  * @retval : PASSED if the reception is well done, FAILED in other case
  */
void bsp_InitCAN1(void)
{
	GPIO_InitTypeDef				GPIO_InitStructure;
	CAN_InitTypeDef					CAN_InitStructure;
  CAN_FilterInitTypeDef		CAN_FilterInitStructure;
	
	/* ʹ������ʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH | RCC_AHB1Periph_GPIOI, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource13, GPIO_AF_CAN1);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource9, GPIO_AF_CAN1);
	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//�������Ϊ����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		//�ڲ���������ʹ��
	//CAN1_RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOI, &GPIO_InitStructure);
	//CAN1_TX	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		//����ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOH, &GPIO_InitStructure);
	
	/* CAN ��λ*/
	CAN_DeInit(CAN1);
	CAN_StructInit(&CAN_InitStructure);
	
	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM=DISABLE;	//ʱ�䴥��
	CAN_InitStructure.CAN_ABOM=DISABLE;	//�Զ����߹���
	CAN_InitStructure.CAN_AWUM=DISABLE;	//�Զ�����
	CAN_InitStructure.CAN_NART=DISABLE;	//�Զ��ش�(ע��,DISABLE�����Զ��ش�)

	CAN_InitStructure.CAN_RFLM=DISABLE;	//ʧ��FIFO����ģʽ
	CAN_InitStructure.CAN_TXFP=DISABLE;	//ʧ��FIFO���ȼ�
	CAN_InitStructure.CAN_Mode=CAN_Mode_LoopBack;//��������ģʽ
	CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1=CAN_BS1_8tq;
	CAN_InitStructure.CAN_BS2=CAN_BS2_7tq;
	CAN_InitStructure.CAN_Prescaler=9;  //48Mhz/(1+8+7)/10 = 300Kbits
	CAN_Init(CAN1,&CAN_InitStructure);

	/* CAN ���������� */
	CAN_FilterInitStructure.CAN_FilterNumber=0;      //���������(0-13)
	CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask; //ָ���˹�����������ʼ������ģʽΪ��ʶ���б�ģʽ
	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit; //�����˹�����λ��1��32λ������
	CAN_FilterInitStructure.CAN_FilterIdHigh=(uint16_t)0x00AA;//�����趨��������ʶ����32λλ��ʱΪ��߶�λ��16λλ��ʱΪ��һ����
	CAN_FilterInitStructure.CAN_FilterIdLow=(uint16_t)0x0000;	//�����趨��������ʶ����32λλ��ʱΪ��Ͷ�λ��16λλ��ʱΪ�ڶ���
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=(uint16_t)0x00FF;//�����趨���������α�ʶ�����߹�������ʶ����32λλ��ʱΪ��߶�λ��16λλ��ʱΪ��һ��
	CAN_FilterInitStructure.CAN_FilterMaskIdLow=(uint16_t)0x0000;//�����趨���������α�ʶ�����߹�������ʶ����32λλ��ʱΪ��Ͷ�λ��16λλ��ʱΪ�ڶ���
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0; //�趨��ָ���������FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;//ʹ�ܹ�����
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	CAN_ITConfig(CAN1,CAN_IT_FMP0, ENABLE);
}
void bsp_InitCAN2(void)
{

}

void CAN1_RX0_IRQHandler()
{
	CanRxMsg	RxMessage;
	CPU_SR		cpu_sr;
	
	CPU_CRITICAL_ENTER();  
	OSIntNestingCtr++;
	CPU_CRITICAL_EXIT();
	
	RxMessage.ExtId = 0;
	RxMessage.StdId = 0;
	CAN_Receive(CAN1,CAN_FIFO0, &RxMessage);
	printf("%d\n\r",RxMessage.StdId);
	printf("%d\n\r",RxMessage.ExtId);
	if(RxMessage.ExtId == 0x00AA0000 )
	{
		printf("%d\n\r",RxMessage.Data[0]);
		printf("%d\n\r",RxMessage.Data[1]);
	}
	OSIntExit();
}
