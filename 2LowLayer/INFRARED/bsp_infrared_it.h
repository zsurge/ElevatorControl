/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_infrared_it.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��7��24��
  ����޸�   :
  ��������   : ���ⴥ�����������жϷ�ʽ����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��7��24��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __bsp_INFRARED_IT_
#define __bsp_INFRARED_IT_

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "delay.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define RCC_ALL_SENSOR     (RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC)
            
#define GPIO_PORT_SENSOR14    GPIOA
#define GPIO_PORT_SENSOR56    GPIOC
#define GPIO_PORT_SENSOR78    GPIOB
        
        
            
#define GPIO_PIN_SENSOR1    GPIO_Pin_4
#define GPIO_PIN_SENSOR2    GPIO_Pin_5
#define GPIO_PIN_SENSOR3    GPIO_Pin_6
#define GPIO_PIN_SENSOR4    GPIO_Pin_7
        
#define GPIO_PIN_SENSOR5    GPIO_Pin_4
#define GPIO_PIN_SENSOR6    GPIO_Pin_5
        
#define GPIO_PIN_SENSOR7    GPIO_Pin_0
#define GPIO_PIN_SENSOR8    GPIO_Pin_1


#define GETS1 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR14,GPIO_PIN_SENSOR1) 
#define GETS2 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR14,GPIO_PIN_SENSOR2)
#define GETS3 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR14,GPIO_PIN_SENSOR3) 
#define GETS4 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR14,GPIO_PIN_SENSOR4)

#define GETS5 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR56,GPIO_PIN_SENSOR5) 
#define GETS6 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR56,GPIO_PIN_SENSOR6)

#define GETS7 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR78,GPIO_PIN_SENSOR7) 
#define GETS8 	GPIO_ReadInputDataBit(GPIO_PORT_SENSOR78,GPIO_PIN_SENSOR8)



/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
extern __IO int16_t g_infraredIRQ;
/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/



void bsp_InitInfraredExit(void);

int16_t bsp_InfraredExitScan(void);





#endif

