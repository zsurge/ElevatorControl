/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : test.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��28��
  ����޸�   :
  ��������   : ����ģ�����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��28��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __TEST_H
#define __TEST_H

//��׼��ͷ�ļ�
#include "stdlib.h"

#include "cJSON.h"


//�м��ͷ�ļ�
#include "sys.h"
#include "delay.h"
#include "easyflash.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sfud.h"
#include "ini.h"


//������ͷ�ļ�
//#include "bsp_usart1.h"
//#include "bsp_usart2.h"
#include "bsp_time.h"
#include "bsp_led.h"
#include "bsp_key.h"

#include "bsp_spi_flash.h"





#define MY_DEBUGF
#ifdef MY_DEBUGF  
#define MY_DEBUG(message,...) do { \
                                  {UARTprintf(message,##__VA_ARGS__);}\
                               } while(0)  
#else    
#define MY_DEBUG(message,...)    
#endif /* PLC_DEBUG */




void COMProcessHelp(void);
void FlashTest(void);
void test_env(void);

//uint8_t SPI_Flash_Test(void);


int CJSON_CDECL json_test(void);
//void spi_flash_demo(void);


#endif


