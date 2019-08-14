/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_usart1.h
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

#ifndef __BSP_USART1_H
#define __BSP_USART1_H

//���Ŷ���
/*******************************************************/
#define USART1_CLK                         RCC_APB2Periph_USART1
#define USART1_GPIO_PORT                   GPIOB
#define USART1_GPIO_CLK                    RCC_AHB1Periph_GPIOB

#define USART1_RX_PIN                      GPIO_Pin_6
#define USART1_RX_AF                       GPIO_AF_USART1
#define USART1_RX_SOURCE                   GPIO_PinSource10

#define USART1_TX_PIN                      GPIO_Pin_7
#define USART1_TX_AF                       GPIO_AF_USART1
#define USART1_TX_SOURCE                   GPIO_PinSource9





#include "stdio.h"	
#include "string.h"
#include "stm32f4xx_conf.h"
#include "stdarg.h"


#define USART1MAXBUFFSIZE 1024 

extern volatile uint8_t USART1RecvBuf[USART1MAXBUFFSIZE];
extern volatile uint16_t RecvTop1;
extern volatile uint16_t RecvEnd1;



void bsp_Usart1_Init (uint32_t BaudRate);
void bsp_Usart1_SendString (const uint8_t *Buff);
void bsp_Usart1_SendData (const uint8_t *Buff, uint16_t SendSize);
void bsp_Usart1_RecvReset (void);
uint8_t bsp_Usart1_RecvOne (uint8_t *Str);
uint16_t bsp_Usart1_RecvAtTime (uint8_t *Buff, uint16_t RecvSize, uint32_t timeout_MilliSeconds);
uint8_t bsp_Usart1_Read(uint8_t *buf, uint16_t len);



    
#endif


