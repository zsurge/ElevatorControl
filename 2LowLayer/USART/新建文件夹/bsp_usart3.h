/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_usart3.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��22��
  ����޸�   :
  ��������   : ����3 ָ�����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��22��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __BSP_USART3_H
#define __BSP_USART3_H

#include "stdio.h"	
#include "string.h"
#include "stm32f4xx_conf.h"

#define USART3_CLK                         RCC_APB1Periph_USART3
#define USART3_GPIO_PORT                   GPIOD
#define USART3_GPIO_CLK                    RCC_AHB1Periph_GPIOD

#define USART3_RX_PIN                      GPIO_Pin_9
#define USART3_RX_AF                       GPIO_AF_USART3
#define USART3_RX_SOURCE                   GPIO_PinSource9


#define USART3_TX_PIN                      GPIO_Pin_8
#define USART3_TX_AF                       GPIO_AF_USART3
#define USART3_TX_SOURCE                   GPIO_PinSource8




#define USART3MAXBUFFSIZE 1024




void bsp_Usart3_Init (uint32_t BaudRate);
void bsp_Usart3_SendString (const uint8_t *Buff);
void bsp_Usart3_SendData (const uint8_t *Buff, uint16_t SendSize);
void bsp_Usart3_RecvReset (void);
uint8_t bsp_Usart3_RecvOne (uint8_t *Str);
uint16_t bsp_Usart3_RecvAtTime (uint8_t *Buff, uint16_t RecvSize, uint32_t timeout_MilliSeconds);
uint8_t bsp_Usart3_Read(uint8_t *buf, uint16_t len);

    
#endif


