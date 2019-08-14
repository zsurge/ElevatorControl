/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_usart2.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��22��
  ����޸�   :
  ��������   : ����2 ָ�����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

#include "bsp_usart2.h"
#include "bsp_time.h"


volatile u8 USART2RecvBuf[USART2MAXBUFFSIZE] = {0};
volatile u16 RecvTop2 = 0;
volatile u16 RecvEnd2 = 0;



/*****************************************************************************
 �� �� ��  : bsp_Usart2Init
 ��������  : ���ڳ�ʼ��
 �������  : u32 BaudRate  ������
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart2Init (u32 BaudRate);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart2_Init (u32 BaudRate)
{
    //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(USART2_GPIO_CLK,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(USART2_CLK,ENABLE);//ʹ��Usart2ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(USART2_GPIO_PORT,USART2_TX_SOURCE,USART2_TX_AF); //GPIOA2����ΪUSART2
	GPIO_PinAFConfig(USART2_GPIO_PORT,USART2_RX_SOURCE,USART2_RX_AF); //GPIOA3����ΪUSART2
	
	//Usart2�˿�����
	GPIO_InitStructure.GPIO_Pin = USART2_TX_PIN | USART2_RX_PIN; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(USART2_GPIO_PORT,&GPIO_InitStructure); //��ʼ��PA2��PA3

   //Usart2 ��ʼ������
	USART_InitStructure.USART_BaudRate = BaudRate;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure); //��ʼ������2    

	//Usart2 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//����2�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//��������ж�    
	
	USART_Cmd(USART2, ENABLE);  //ʹ�ܴ���1   



}

void USART2_IRQHandler (void)
{
	u8 temp = 0;

	//Receive data register not empty flag
	if (USART_GetITStatus (USART2, USART_IT_RXNE) != RESET)
	{
		temp = USART_ReceiveData (USART2);		//get received data

		if ( (RecvEnd2 == (RecvTop2 - 1) ) || ( (RecvTop2 == 0) && (RecvEnd2 == (USART2MAXBUFFSIZE - 1) ) ) )
		{
			//�������
			RecvTop2 = 0;
			RecvEnd2 = 0;
		}
		else
		{
			USART2RecvBuf[RecvEnd2] = temp;
			RecvEnd2++;

			if (RecvEnd2 >= USART2MAXBUFFSIZE) RecvEnd2 = 0;
		}

		/* Clear the UART1 Recvive interrupt */
		USART_ClearITPendingBit (USART2, USART_IT_RXNE);
	}
	else if (USART_GetITStatus (USART2, USART_IT_NE | USART_IT_PE | USART_IT_FE | USART_IT_ORE) != RESET)
	{
		USART_GetITStatus (USART2, USART_IT_NE | USART_IT_PE | USART_IT_FE | USART_IT_ORE);
		USART_ReceiveData (USART2);
	}
}


void bsp_Usart2_SendOne (const u8 dat)
{

    
	USART_SendData (USART2, dat);

	while (USART_GetFlagStatus (USART2, USART_FLAG_TXE) == RESET) {}

	USART_ClearFlag (USART2, USART_FLAG_TXE);

}



/*****************************************************************************
 �� �� ��  : bsp_Usart2_SendString
 ��������  : �����ַ���
 �������  : const u8 *Buff  �����ַ���
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart2_SendString (const u8 *Buff);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart2_SendString (const u8 *Buff)
{
	while (*Buff != 0)
	{
		bsp_Usart2_SendOne (*Buff);
		Buff++;
	}    
}


/*****************************************************************************
 �� �� ��  : bsp_Usart2SendData
 ��������  : ����ָ�������ַ�
 �������  : const u8 *Buff  �������ݵ�buff
             u16 SendSize  �������ݵĳ���  
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart2SendData (const u8 *Buff, u16 SendSize);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart2_SendData (const u8 *Buff, u16 SendSize)
{
	while (SendSize != 0)
	{
		bsp_Usart2_SendOne (*Buff);
		Buff++;
		SendSize--;
	}

}



/*****************************************************************************
 �� �� ��  : bsp_Usart2RecvReset
 ��������  : ��մ��ڽ��ջ���
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
void bsp_Usart2RecvReset (void);
    �޸�����   : �����ɺ���

*****************************************************************************/
void bsp_Usart2_RecvReset (void)
{
	RecvTop2 = 0;
	RecvEnd2 = 0;

	memset((void *)USART2RecvBuf, 0, USART2MAXBUFFSIZE); 
}



/*****************************************************************************
 �� �� ��  : bsp_Usart2RecvOne
 ��������  : ��ȡ1���ֽ�
 �������  : u8 *Str  �������ֽڴ洢��Str[0]
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
u8 bsp_Usart2RecvOne (u8 *Str);
    �޸�����   : �����ɺ���

*****************************************************************************/
u8 bsp_Usart2_RecvOne (u8 *Str)
{
	if (RecvTop2 == RecvEnd2) return 0;//read nothing

	*Str = USART2RecvBuf[RecvTop2];
	RecvTop2++;

	if (RecvTop2 >= USART2MAXBUFFSIZE) RecvTop2 = 0;

	return 1;//read one

}



/*****************************************************************************
 �� �� ��  : bsp_Usart2RecvAtTime
 ��������  : ���մ�������
 �������  : u8 *Buff �������ݴ�ŵĵط�                 
             u16 RecvSize  ���ܶ��ٸ��ֽ�            
             u32 timeout_MilliSeconds  ��ʱ�ĺ�����
 �������  : ��
 �� �� ֵ  : ���ؽ��ܵ����ֽ���
 
 �޸���ʷ      :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
u16 bsp_Usart2RecvAtTime (u8 *Buff, u16 RecvSize, u32 timeout_MilliSeconds);
    �޸�����   : �����ɺ���

*****************************************************************************/
u16 bsp_Usart2_RecvAtTime (u8 *Buff, u16 RecvSize, u32 timeout_MilliSeconds)
{
	u16 RecvLen = 0;
	u8 tmp[1] = {0};

	if (RecvSize == 0) return 0;

	g1msTimerUART2 = timeout_MilliSeconds;

	while (1)
	{
		if (g1msTimerUART2 == 0) return RecvLen;

		if (bsp_Usart2_RecvOne (tmp) == 1) 
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
u8 bsp_Usart2_Read(u8 *Buff, u16 len)
{

	u16 RecvLen = 0;
	u8 tmp[1] = {0};

	if (len == 0 || Buff == NULL) return 0;

	while (len--)
	{
		if (bsp_Usart2_RecvOne (tmp) == 1)
		{
			Buff[RecvLen++] = tmp[0];
		}

		if (RecvLen >= len) return RecvLen;
	}

	return RecvLen;
    
}








