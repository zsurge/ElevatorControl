#include "bsp_rs485.h"	 
#include "delay.h"
#include "bsp_time.h"


//���ջ����� 	
static uint8_t RS485_RX_BUF[RS485_MAX_LEN];  	//���ջ���,���255���ֽ�.
//���յ������ݳ���
static uint8_t RS485_RX_CNT=0;  




void UART5_IRQHandler(void)
{
	uint8_t res;	  

	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)//���յ�����
	{	 	
        
	    res =USART_ReceiveData(UART5);//;��ȡ���յ�������UART5->DR

        USART_ClearITPendingBit(UART5, USART_IT_RXNE);  // ���UART5�Ľ����ж� �� 
		if(RS485_RX_CNT < RS485_MAX_LEN)
		{
			RS485_RX_BUF[RS485_RX_CNT]=res;		//��¼���յ���ֵ
			RS485_RX_CNT++;						//������������1 
		} 
	}  											 
} 


								 
//��ʼ��IO ����2
//bound:������	  
void bsp_RS485_Init(uint32_t bound)
{  	 
	
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(USART5_GPIO_RX_CLK|USART5_GPIO_TX_CLK|RCC_RS485_TXEN,ENABLE); //ʹ��GPIOʱ��
	RCC_APB1PeriphClockCmd(USART5_CLK,ENABLE);//ʹ��UART5ʱ��
	
  //����5���Ÿ���ӳ��
	GPIO_PinAFConfig(USART5_GPIO_TX_PORT,USART5_TX_SOURCE,USART5_AF); //GPIOC12����ΪUART5
	GPIO_PinAFConfig(USART5_GPIO_RX_PORT,USART5_RX_SOURCE,USART5_AF); //GPIOD2����ΪUART5
	
	//UART5    
    GPIO_InitStructure.GPIO_Pin = USART5_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(USART5_GPIO_TX_PORT,&GPIO_InitStructure); //��ʼ��


    GPIO_InitStructure.GPIO_Pin = USART5_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(USART5_GPIO_RX_PORT,&GPIO_InitStructure); //��ʼ��
	
	
	//PG0���������485ģʽ����  
    GPIO_InitStructure.GPIO_Pin = PIN_RS485_TXEN; //GPIOG0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(PORT_RS485_TXEN,&GPIO_InitStructure); //��ʼ��PE2
	

   //UART5 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
    USART_Init(UART5, &USART_InitStructure); //��ʼ������2
	
    USART_Cmd(UART5, ENABLE);  //ʹ�ܴ��� 2
	
	USART_ClearFlag(UART5, USART_FLAG_TC);
	

	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//���������ж�

	//Usart2 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	RS485_TX_EN=0;				//Ĭ��Ϊ����ģʽ	
}

//RS485����len���ֽ�.
//buf:�������׵�ַ
//len:���͵��ֽ���(Ϊ�˺ͱ�����Ľ���ƥ��,���ｨ�鲻Ҫ����64���ֽ�)
void bsp_RS485_Send_Data(uint8_t *buf,uint16_t len)
{
	uint8_t t;
	RS485_TX_EN=1;			//����Ϊ����ģʽ
	
  	for(t=0;t<len;t++)		//ѭ����������
	{
	    while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==RESET); //�ȴ����ͽ���		
        USART_SendData(UART5,buf[t]); //��������
	}	
    
	while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==RESET); //�ȴ����ͽ���		
	
	RS485_RX_CNT=0;	  
	RS485_TX_EN=0;				//����Ϊ����ģʽ	
}


//RS485��ѯ���յ�������
//buf:���ջ����׵�ַ
//len:���������ݳ���
void bsp_RS485_Receive_Data(uint8_t *buf,uint16_t *len)
{
	uint8_t rxlen=RS485_RX_CNT;
	uint8_t i=0;
	*len=0;				//Ĭ��Ϊ0
	delay_ms(10);		//�ȴ�10ms,��������10msû�н��յ�һ������,����Ϊ���ս���    
    
	if(rxlen==RS485_RX_CNT&&rxlen)//���յ�������,�ҽ��������
	{
		for(i=0;i<rxlen;i++)
		{
			buf[i]=RS485_RX_BUF[i];	
		}		
		*len=RS485_RX_CNT;	//��¼�������ݳ���
		RS485_RX_CNT=0;		//����
	}
}

//static uint8_t RS485_RecvOne (uint8_t *Str)
//{
//	if (RecvTop2 == RS485_RX_CNT) return 0;//read nothing

//	*Str = RS485_RX_BUF[RecvTop2];
//	RecvTop2++;

//	if (RecvTop2 >= RS485_MAX_LEN) RecvTop2 = 0;

//	return 1;//read one


//}


//uint16_t bsp_RS485_Receive_Data (uint8_t *buf,uint16_t len,uint32_t timeout_MilliSeconds)
//{
//	u16 RecvLen = 0;
//	u8 tmp[1] = {0};

//	if (len == 0) return 0;

//	g1msTimerUART2 = timeout_MilliSeconds;

//	while (1)
//	{
//		if (g1msTimerUART2 == 0) return RecvLen;

//		if (RS485_RecvOne (tmp) == 1) 
//		{
//			buf[RecvLen++] = tmp[0];
//		}

//		if (RecvLen >= len) return RecvLen;
//	}
//	
//}






