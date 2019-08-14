/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_Usart3.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��22��
  ����޸�   :
  ��������   : ����3 IAP����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

#include "bsp_usart4.h"
#include "bsp_time.h"


volatile uint8_t USART4RecvBuf[UART4MAXBUFFSIZE] = {0};
volatile uint16_t RecvTop4 = 0;
volatile uint16_t RecvEnd4 = 0;



/*****************************************************************************
 �� �� ��  : bsp_Usart3Init
 ��������  : ���ڳ�ʼ��
 �������  : uint32_t BaudRate  ������
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart3Init (uint32_t BaudRate);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart4_Init (uint32_t BaudRate)
{
    //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(UART4_GPIO_CLK,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(UART4_CLK,ENABLE);//ʹ��Usart3ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(UART4_GPIO_PORT,UART4_TX_SOURCE,UART4_TX_AF); //GPIOB10����ΪUSART3
	GPIO_PinAFConfig(UART4_GPIO_PORT,UART4_RX_SOURCE,UART4_RX_AF); //GPIOB11����ΪUSART3
	
	
	//Usart2�˿�����
	GPIO_InitStructure.GPIO_Pin = UART4_RX_PIN|UART4_TX_PIN; //GPIOB10��GPIOB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(UART4_GPIO_PORT,&GPIO_InitStructure); //��ʼ��PB10��PB11
	

   //UART4 ��ʼ������
	USART_InitStructure.USART_BaudRate = BaudRate;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(UART4, &USART_InitStructure); //��ʼ������2    

	//UART4 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;//����3�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//��������ж�    
	
	USART_Cmd(UART4, ENABLE);  //ʹ�ܴ���1   

//	init_serial_boot();//��ʼ�����ڻ�����
}

void UART4_IRQHandler(void)
{
	uint8_t temp = 0;

	//Receive data register not empty flag
	if (USART_GetITStatus (UART4, USART_IT_RXNE) != RESET)
	{
		temp = USART_ReceiveData (UART4);		//get received data

		if ( (RecvEnd4 == (RecvTop4 - 1) ) || ( (RecvTop4 == 0) && (RecvEnd4 == (UART4MAXBUFFSIZE - 1) ) ) )
		{
			//�������
			RecvTop4 = 0;
			RecvEnd4 = 0;
		}
		else
		{
			USART4RecvBuf[RecvEnd4] = temp;
			RecvEnd4++;

			if (RecvEnd4 >= UART4MAXBUFFSIZE) RecvEnd4 = 0;
		}

		/* Clear the UART1 Recvive interrupt */
		USART_ClearITPendingBit (UART4, USART_IT_RXNE);
	}
	else if (USART_GetITStatus (UART4, USART_IT_NE | USART_IT_PE | USART_IT_FE | USART_IT_ORE) != RESET)
	{
		USART_GetITStatus (UART4, USART_IT_NE | USART_IT_PE | USART_IT_FE | USART_IT_ORE);
		USART_ReceiveData (UART4);
	}
}


void bsp_Usart4_SendOne (const uint8_t dat)
{
	USART_SendData (UART4, dat);

	while (USART_GetFlagStatus (UART4, USART_FLAG_TXE) == RESET) {}

	USART_ClearFlag (UART4, USART_FLAG_TXE);

}



/*****************************************************************************
 �� �� ��  : bsp_Usart3SendString
 ��������  : �����ַ���
 �������  : const uint8_t *Buff  �����ַ���
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart3SendString (const uint8_t *Buff);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart4_SendString (const uint8_t *Buff)
{
	while (*Buff != 0)
	{
		bsp_Usart4_SendOne (*Buff);
		Buff++;
	}    
}


/*****************************************************************************
 �� �� ��  : bsp_Usart3SendData
 ��������  : ����ָ�������ַ�
 �������  : const uint8_t *Buff  �������ݵ�buff
             uint16_t SendSize  �������ݵĳ���  
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart3SendData (const uint8_t *Buff, uint16_t SendSize);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart4_SendData (const uint8_t *Buff, uint16_t SendSize)
{
	while (SendSize != 0)
	{
		bsp_Usart4_SendOne (*Buff);
		Buff++;
		SendSize--;
	}

}



/*****************************************************************************
 �� �� ��  : bsp_Usart3RecvReset
 ��������  : ��մ��ڽ��ջ���
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart3RecvReset (void);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart4_RecvReset (void)
{
	RecvTop4 = 0;
	RecvEnd4 = 0;

	memset((void *)USART4RecvBuf, 0, UART4MAXBUFFSIZE); 
}



/*****************************************************************************
 �� �� ��  : bsp_Usart3RecvOne
 ��������  : ��ȡ1���ֽ�
 �������  : uint8_t *Str  �������ֽڴ洢��Str[0]
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
uint8_t bsp_Usart3RecvOne (uint8_t *Str);
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t bsp_Usart4_RecvOne (uint8_t *Str)
{
	if (RecvTop4 == RecvEnd4) return 0;//read nothing

	*Str = USART4RecvBuf[RecvTop4];
	RecvTop4++;

	if (RecvTop4 >= UART4MAXBUFFSIZE) RecvTop4 = 0;

	return 1;//read one

}



/*****************************************************************************
 �� �� ��  : bsp_Usart3RecvAtTime
 ��������  : ���մ�������
 �������  : uint8_t *Buff �������ݴ�ŵĵط�                 
             uint16_t RecvSize  ���ܶ��ٸ��ֽ�            
             uint32_t timeout_MilliSeconds  ��ʱ�ĺ�����
 �������  : ��
 �� �� ֵ  : ���ؽ��ܵ����ֽ���
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
uint16_t bsp_Usart3RecvAtTime (uint8_t *Buff, uint16_t RecvSize, uint32_t timeout_MilliSeconds);
    �޸�����   : �����ɺ���

*****************************************************************************/
uint16_t bsp_Usart4_RecvAtTime (uint8_t *Buff, uint16_t RecvSize, uint32_t timeout_MilliSeconds)
{
	uint16_t RecvLen = 0;
	uint8_t tmp[1] = {0};

	if (RecvSize == 0) return 0;

	g1msTimerUART4 = timeout_MilliSeconds;

	while (1)
	{
		if (g1msTimerUART4 == 0) return RecvLen;

		if (bsp_Usart4_RecvOne (tmp) == 1) 
		{
			Buff[RecvLen++] = tmp[0];
		}

		if (RecvLen >= RecvSize) return RecvLen;
	}
	
	
}


/*****************************************************************************
 �� �� ��  : dev_Usart2Read
 ��������  : ����������
 �������  : unsigned char *buf  
             int len             
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
int dev_Usart2Read(unsigned char *buf, int len)
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t bsp_Usart4_Read(uint8_t *Buff, uint16_t len)
{

	uint16_t RecvLen = 0;
	uint8_t tmp[1] = {0};

	if (len == 0 || Buff == NULL) return 0;

	while (len--)
	{
		if (bsp_Usart4_RecvOne (tmp) == 1)
		{
			Buff[RecvLen++] = tmp[0];
		}

		if (RecvLen >= len) return RecvLen;
	}

	return RecvLen;
    
}



