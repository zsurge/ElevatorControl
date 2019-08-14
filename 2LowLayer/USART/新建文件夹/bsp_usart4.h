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

#ifndef __BSP_USART4_H
#define __BSP_USART4_H

#include "stdio.h"	
#include "string.h"
#include "stm32f4xx_conf.h"

#define UART4_CLK                         RCC_APB1Periph_UART4
#define UART4_GPIO_PORT                   GPIOC
#define UART4_GPIO_CLK                    RCC_AHB1Periph_GPIOC

#define UART4_RX_PIN                      GPIO_Pin_11
#define UART4_RX_AF                       GPIO_AF_UART4
#define UART4_RX_SOURCE                   GPIO_PinSource11


#define UART4_TX_PIN                      GPIO_Pin_10
#define UART4_TX_AF                       GPIO_AF_UART4
#define UART4_TX_SOURCE                   GPIO_PinSource10


#define UART4MAXBUFFSIZE 1024

void bsp_Usart4_Init (uint32_t BaudRate);
void bsp_Usart4_SendString (const uint8_t *Buff);
void bsp_Usart4_SendData (const uint8_t *Buff, u16 SendSize);
void bsp_Usart4_RecvReset (void);
uint8_t bsp_Usart4_RecvOne (uint8_t *Str);
u16 bsp_Usart4_RecvAtTime (uint8_t *Buff, u16 RecvSize, uint32_t timeout_MilliSeconds);
uint8_t bsp_Usart4_Read(uint8_t *buf, u16 len);

    
#endif


