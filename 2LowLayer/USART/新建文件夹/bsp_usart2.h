/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : drv_usart2.h
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

#ifndef __DRV_USART2_H
#define __DRV_USART2_H


#include "stdio.h"	
#include "string.h"
#include "stm32f4xx_conf.h"

//���Ŷ���
/*******************************************************/
#define USART2_CLK                         RCC_APB1Periph_USART2
#define USART2_GPIO_PORT                   GPIOA
#define USART2_GPIO_CLK                    RCC_AHB1Periph_GPIOA

#define USART2_RX_PIN                      GPIO_Pin_3
#define USART2_RX_AF                       GPIO_AF_USART2
#define USART2_RX_SOURCE                   GPIO_PinSource3

#define USART2_TX_PIN                      GPIO_Pin_2
#define USART2_TX_AF                       GPIO_AF_USART2
#define USART2_TX_SOURCE                   GPIO_PinSource2





#define USART2MAXBUFFSIZE 1024

void bsp_Usart2_Init (u32 BaudRate);
void bsp_Usart2_SendString (const u8 *Buff);
void bsp_Usart2_SendData (const u8 *Buff, u16 SendSize);
void bsp_Usart2_RecvReset (void);
u8 bsp_Usart2_RecvOne (u8 *Str);
u16 bsp_Usart2_RecvAtTime (u8 *Buff, u16 RecvSize, u32 timeout_MilliSeconds);
u8 bsp_Usart2_Read(u8 *buf, u16 len);


    
#endif


