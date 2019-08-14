#ifndef __BSP_LED_H
#define __BSP_LED_H
#include "sys.h"


//LED�˿ڶ���
#define RCC_ALL_LED     (RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOD)

#define GPIO_PORT_ERRORLED      GPIOA
#define GPIO_PORT_LED2          GPIOD
#define GPIO_PORT_LED134        GPIOF

#define GPIO_PIN_ERRORLED   GPIO_Pin_8
#define GPIO_PIN_LED1       GPIO_Pin_6
#define GPIO_PIN_LED2       GPIO_Pin_13
#define GPIO_PIN_LED3       GPIO_Pin_7
#define GPIO_PIN_LED4       GPIO_Pin_9

//ͨ��ָʾ�ƶ���
#define RCC_EX_LED     (RCC_AHB1Periph_GPIOF)

#define GPIO_PORT_EX_LED    GPIOF

#define GPIO_PIN_L_R    GPIO_Pin_7     //�����
#define GPIO_PIN_L_G    GPIO_Pin_6     //����̵�
#define GPIO_PIN_M_R    GPIO_Pin_9     //�м���
#define GPIO_PIN_M_G    GPIO_Pin_8     //�м��̵�
#define GPIO_PIN_R_R    GPIO_Pin_5     //�Ҳ���
#define GPIO_PIN_R_G    GPIO_Pin_4     //�Ҳ��̵�


#define LED_L_R    PFout(7)      //�����
#define LED_L_G    PFout(6)     //����̵�
#define LED_M_R    PFout(9)     //�м���
#define LED_M_G    PFout(8)     //�м��̵�
#define LED_R_R    PFout(5)     //�Ҳ���
#define LED_R_G    PFout(4)     //�Ҳ��̵�


#define LEDERROR PAout(8)	
#define LED1 PFout(6)	
#define LED2 PDout(13)	 
#define LED3 PFout(7)	 
#define LED4 PFout(9)	 	


#define RL3 PFout(0)
#define RL4 PFout(1)

#define LOCK_STATUS	GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_3)



void bsp_LED_Init(void);//��ʼ��		 	
void bsp_Ex_LED_Init(void); //��չFLASH��ʼ��
void bsp_Ex_SetLed(uint8_t *dat);





void bsp_OTHER_test_Init(void);


void bsp_lock_init(void);



//void bsp_LedToggle(uint8_t _no);
#endif
