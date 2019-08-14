/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_usart1.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��22��
  ����޸�   :
  ��������   : ����1��ʼ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#include "stm32f4xx_conf.h"
#include "bsp_usart1.h"
#include "bsp_time.h"

#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
	USART1->DR = (u8) ch;      
	return ch;

}

//__use_no_semihosting was requested, but _ttywrch was 
//int _ttywrch(int ch)
//{
//    ch = ch;
//}

#endif




volatile uint8_t USART1RecvBuf[USART1MAXBUFFSIZE] = {0};
volatile uint16_t RecvTop1 = 0;
volatile uint16_t RecvEnd1 = 0;

/*****************************************************************************
 �� �� ��  : bsp_Usart1Init
 ��������  : ���ڳ�ʼ��
 �������  : uint32_t BaudRate  ������
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart1Init (uint32_t BaudRate);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart1_Init (uint32_t BaudRate)
{
    //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(USART1_GPIO_CLK,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(USART1_CLK,ENABLE);//ʹ��USART1ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(USART1_GPIO_PORT,USART1_TX_SOURCE,USART1_TX_AF); //GPIOA9����ΪUSART1
	GPIO_PinAFConfig(USART1_GPIO_PORT,USART1_RX_SOURCE,USART1_RX_AF); //GPIOA10����ΪUSART1
	
	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = USART1_TX_PIN | USART1_RX_PIN; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(USART1_GPIO_PORT,&GPIO_InitStructure); //��ʼ��PA9��PA10

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = BaudRate;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART1, &USART_InitStructure); //��ʼ������1    

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;		//�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//��������ж�    
	
	USART_Cmd(USART1, ENABLE);  //ʹ�ܴ���1   

}

void USART1_IRQHandler (void)
{
	uint8_t temp = 0;

	//Receive data register not empty flag
	if (USART_GetITStatus (USART1, USART_IT_RXNE) != RESET)
	{
		temp = USART_ReceiveData (USART1);		//get received data

		if ( (RecvEnd1 == (RecvTop1 - 1) ) || ( (RecvTop1 == 0) && (RecvEnd1 == (USART1MAXBUFFSIZE - 1) ) ) )
		{
			//�������
			RecvTop1 = 0;
			RecvEnd1 = 0;
		}
		else
		{
			USART1RecvBuf[RecvEnd1] = temp;
			RecvEnd1++;

			if (RecvEnd1 >= USART1MAXBUFFSIZE) RecvEnd1 = 0;
		}

		/* Clear the UART1 Recvive interrupt */
		USART_ClearITPendingBit (USART1, USART_IT_RXNE);
	}
	else if (USART_GetITStatus (USART1, USART_IT_NE | USART_IT_PE | USART_IT_FE | USART_IT_ORE) != RESET)
	{
		USART_GetITStatus (USART1, USART_IT_NE | USART_IT_PE | USART_IT_FE | USART_IT_ORE);
		USART_ReceiveData (USART1);
	}
}


void bsp_Usart1_SendOne (const uint8_t dat)
{
	USART_SendData (USART1, dat);

	while (USART_GetFlagStatus (USART1, USART_FLAG_TXE) == RESET) {}

	USART_ClearFlag (USART1, USART_FLAG_TXE);

}



/*****************************************************************************
 �� �� ��  : bsp_Usart1SendString
 ��������  : �����ַ���
 �������  : const uint8_t *Buff  �����ַ���
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart1SendString (const uint8_t *Buff);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart1_SendString (const uint8_t *Buff)
{
	while (*Buff != 0)
	{
		bsp_Usart1_SendOne (*Buff);
		Buff++;
	}    
}


/*****************************************************************************
 �� �� ��  : bsp_Usart1SendData
 ��������  : ����ָ�������ַ�
 �������  : const uint8_t *Buff  �������ݵ�buff
             uint16_t SendSize  �������ݵĳ���  
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart1SendData (const uint8_t *Buff, uint16_t SendSize);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart1_SendData (const uint8_t *Buff, uint16_t SendSize)
{
	while (SendSize != 0)
	{
		bsp_Usart1_SendOne (*Buff);
		Buff++;
		SendSize--;
	}

}



/*****************************************************************************
 �� �� ��  : bsp_Usart1RecvReset
 ��������  : ��մ��ڽ��ջ���
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart1RecvReset (void);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart1_RecvReset (void)
{
	RecvTop1 = 0;
	RecvEnd1 = 0;

	memset((void *)USART1RecvBuf, 0, USART1MAXBUFFSIZE); 
}



/*****************************************************************************
 �� �� ��  : bsp_Usart1RecvOne
 ��������  : ��ȡ1���ֽ�
 �������  : uint8_t *Str  �������ֽڴ洢��Str[0]
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
uint8_t bsp_Usart1RecvOne (uint8_t *Str);
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t bsp_Usart1_RecvOne (uint8_t *Str)
{
	if (RecvTop1 == RecvEnd1) return 0;//read nothing

	*Str = USART1RecvBuf[RecvTop1];
	RecvTop1++;

	if (RecvTop1 >= USART1MAXBUFFSIZE) RecvTop1 = 0;

	return 1;//read one

}



/*****************************************************************************
 �� �� ��  : bsp_USART1RecvAtTime
 ��������  : ���մ�������
 �������  : uint8_t *Buff �������ݴ�ŵĵط�                 
             uint16_t RecvSize  ���ܶ��ٸ��ֽ�            
             uint32_t timeout_MilliSeconds  ��ʱ�ĺ�����
 �������  : ��
 �� �� ֵ  : ���ؽ��ܵ����ֽ���
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
uint16_t bsp_USART1RecvAtTime (uint8_t *Buff, uint16_t RecvSize, uint32_t timeout_MilliSeconds);
    �޸�����   : �����ɺ���

*****************************************************************************/
uint16_t bsp_Usart1_RecvAtTime (uint8_t *Buff, uint16_t RecvSize, uint32_t timeout_MilliSeconds)
{
	uint16_t RecvLen = 0;
	uint8_t tmp[1] = {0};

	if (RecvSize == 0) return 0;

	g1msTimerUART1 = timeout_MilliSeconds;

	while (1)
	{
		if (g1msTimerUART1 == 0) return RecvLen;

		if (bsp_Usart1_RecvOne (tmp) == 1) 
		{
			Buff[RecvLen++] = tmp[0];
		}

		if (RecvLen >= RecvSize) return RecvLen;
	}
	return 0;
}


/*****************************************************************************
 �� �� ��  : dev_Usart1Read
 ��������  : ����������
 �������  : unsigned char *buf  
             int len             
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
int dev_Usart1Read(unsigned char *buf, int len)
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t bsp_Usart1_Read(uint8_t *Buff, uint16_t len)
{

	uint16_t RecvLen = 0;
	uint8_t tmp[1] = {0};

	if (len == 0 || Buff == NULL) return 0;

	while (len--)
	{
		if (bsp_Usart1_RecvOne (tmp) == 1)
		{
			Buff[RecvLen++] = tmp[0];
		}

		if (RecvLen >= len) return RecvLen;
	}

	return RecvLen;
    
}






