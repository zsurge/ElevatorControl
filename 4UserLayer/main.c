/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : main.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��7��9��
  ����޸�   :
  ��������   : ������ģ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��7��9��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/


#include "INCLUDES.h"

#include "client.h"
#include "tcp_server_demo.h"

#include "mqtt_app.h"

#define LOG_TAG    "main"
#include "elog.h"


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
//�������ȼ�   

#define APP_TASK_PRIO		( tskIDLE_PRIORITY)
#define CMD_TASK_PRIO		( tskIDLE_PRIORITY)
#define LWIP_TASK_PRIO		( tskIDLE_PRIORITY + 1)
#define LED_TASK_PRIO		( tskIDLE_PRIORITY + 1)
#define INFRARED_TASK_PRIO	( tskIDLE_PRIORITY + 3)
#define MOTOR_TASK_PRIO		( tskIDLE_PRIORITY + 3)
#define RS485_TASK_PRIO	    ( tskIDLE_PRIORITY + 2)
#define READER_TASK_PRIO	( tskIDLE_PRIORITY + 2)
#define QR_TASK_PRIO	    ( tskIDLE_PRIORITY + 2)
#define KEY_TASK_PRIO	    ( tskIDLE_PRIORITY + 2)
#define START_TASK_PRIO		( tskIDLE_PRIORITY + 4)


//�����ջ��С    
#define LED_STK_SIZE 		128
#define MOTOR_STK_SIZE 		256 
#define CMD_STK_SIZE 		256
#define INFRARED_STK_SIZE 	512
#define RS485_STK_SIZE 		1024*1
#define START_STK_SIZE 	    128
#define QR_STK_SIZE 		1024*2
#define READER_STK_SIZE     512
#define KEY_STK_SIZE        1024*1
#define APP_STK_SIZE 		1024*1
#define LWIP_STK_SIZE 		256


#define  MQTT_QUEUE_LEN    4   /* ���еĳ��ȣ����ɰ������ٸ���Ϣ */
#define  MQTT_QUEUE_SIZE   4   /* ������ÿ����Ϣ��С���ֽڣ� */



//�¼���־
#define TASK_BIT_0	 (1 << 0)
#define TASK_BIT_1	 (1 << 1)
#define TASK_BIT_2	 (1 << 2)
#define TASK_BIT_3	 (1 << 3)
#define TASK_BIT_4	 (1 << 4)
#define TASK_BIT_5	 (1 << 5)
#define TASK_BIT_6	 (1 << 6)


//#define TASK_BIT_ALL (TASK_BIT_0 | TASK_BIT_1 | TASK_BIT_2 | TASK_BIT_3|TASK_BIT_4 | TASK_BIT_5 | TASK_BIT_6 )
#define TASK_BIT_ALL ( TASK_BIT_0 | TASK_BIT_1 | TASK_BIT_2 |TASK_BIT_3|TASK_BIT_4|TASK_BIT_5|TASK_BIT_6)

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
//������
static TaskHandle_t xHandleTaskLed = NULL;      //LED��
static TaskHandle_t xHandleTaskMotor = NULL;    //�������
static TaskHandle_t xHandleTaskCmd = NULL;      //androidͨѶ
static TaskHandle_t xHandleTaskInfrared = NULL; //�����ӳ
static TaskHandle_t xHandleTaskReader = NULL;   //Τ��������
static TaskHandle_t xHandleTaskQr = NULL;       //��ά���ͷ
static TaskHandle_t xHandleTaskRs485 = NULL;
static TaskHandle_t xHandleTaskStart = NULL;    //���Ź�

static TaskHandle_t xHandleTaskKey = NULL;    //����
static TaskHandle_t xHandleTaskLwip = NULL;    //����
static TaskHandle_t xHandleTaskApp = NULL;    ////�����û�����




static SemaphoreHandle_t  xMutex = NULL;
static EventGroupHandle_t xCreatedEventGroup = NULL;
QueueHandle_t MQTT_Data_Queue =NULL;


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

//������
static void vTaskLed(void *pvParameters);
static void vTaskMortorToHost(void *pvParameters);
static void vTaskKey(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskInfrared(void *pvParameters);
static void vTaskRs485(void *pvParameters);
static void vTaskReader(void *pvParameters);
static void vTaskQR(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void vTaskDisplay(void *pvParameters);
static void vTaskLwip(void *pvParameters);
static void vTaskAppCreate(void *pvParameters);



static void show_address(u8 mode);

static void AppObjCreate (void);
static void App_Printf(char *format, ...);
//static void AppEventCreate (void);
static void EasyLogInit(void);

int main(void)
{    
    //Ӳ����ʼ��
    bsp_Init();  

    EasyLogInit();    

	mymem_init(SRAMIN);								//��ʼ���ڲ��ڴ��
	mymem_init(SRAMEX);								//��ʼ���ⲿ�ڴ��
	mymem_init(SRAMCCM);	  					    //��ʼ��CCM�ڴ��

	while(lwip_comm_init() != 0) //lwip��ʼ��
	{
        log_d("lwip init error!\r\n");
		delay_ms(1200);
	}

    log_d("lwip init success!\r\n");

	/* ��������ͨ�Ż��� */
	AppObjCreate(); 

    


    
	//������ʼ����
    xTaskCreate((TaskFunction_t )vTaskAppCreate,            //������
                (const char*    )"AppCreate",          //��������
                (uint16_t       )APP_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )APP_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&xHandleTaskApp);   //������          
                
    /* �������ȣ���ʼִ������ */
    vTaskStartScheduler();
    
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskCreate
*	����˵��: ����Ӧ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void vTaskAppCreate (void *pvParameters)
{

#if LWIP_DHCP
	lwip_comm_dhcp_creat(); 							//����DHCP����
#endif

//    tcp_server_init(); //tcp server demo

//    mqtt_thread_init();

//    client_init();

    taskENTER_CRITICAL();           //�����ٽ���

    //����LED����
    xTaskCreate((TaskFunction_t )vTaskLed,         
                (const char*    )"vTaskLed",       
                (uint16_t       )LED_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )LED_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskLed);   

    //���������Ϣ��������
//    xTaskCreate((TaskFunction_t )vTaskMortorToHost,     
//                (const char*    )"vTMTHost",   
//                (uint16_t       )MOTOR_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )MOTOR_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskMotor);                       

    //���̲���
    xTaskCreate((TaskFunction_t )vTaskKey,     
                (const char*    )"vTaskKey",   
                (uint16_t       )KEY_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )KEY_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskKey);                 
                

    //��androidͨѶ�������ݽ���
//    xTaskCreate((TaskFunction_t )vTaskMsgPro,     
//                (const char*    )"cmd",   
//                (uint16_t       )CMD_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )CMD_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskCmd);      

    //���⴫����״̬����
//    xTaskCreate((TaskFunction_t )vTaskInfrared,     
//                (const char*    )"vTaskInfrared",   
//                (uint16_t       )INFRARED_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )INFRARED_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskInfrared);    


    //485����
//    xTaskCreate((TaskFunction_t )vTaskRs485,     
//                (const char*    )"vTaskRs485",   
//                (uint16_t       )RS485_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )RS485_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskRs485);  

    //Τ��������
//    xTaskCreate((TaskFunction_t )vTaskReader,     
//                (const char*    )"vTaskReader",   
//                (uint16_t       )READER_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )READER_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskReader);   

    xTaskCreate((TaskFunction_t )vTaskLwip,
                (const char*    )"vTaskLwip",   
                (uint16_t       )LWIP_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )LWIP_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskLwip);                     

    //��ά��ɨ��ģ��    
//    xTaskCreate((TaskFunction_t )vTaskDisplay, 
    xTaskCreate((TaskFunction_t )vTaskQR,     
                (const char*    )"vTaskQR",   
                (uint16_t       )QR_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )QR_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskQr);                  

    //���Ź�
//	xTaskCreate((TaskFunction_t )vTaskStart,     		/* ������  */
//                (const char*    )"vTaskStart",   		/* ������    */
//                (uint16_t       )START_STK_SIZE,        /* ����ջ��С����λword��Ҳ����4�ֽ� */
//                (void*          )NULL,           		/* �������  */
//                (UBaseType_t    )START_TASK_PRIO,       /* �������ȼ�*/
//                (TaskHandle_t*  )&xHandleTaskStart );   /* ������  */    

	vTaskDelete(xHandleTaskApp); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���

}


/*
*********************************************************************************************************
*	�� �� ��: AppObjCreate
*	����˵��: ��������ͨ�Ż���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppObjCreate (void)
{
	/* �����¼���־�� */
	xCreatedEventGroup = xEventGroupCreate();
	
	if(xCreatedEventGroup == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
        App_Printf("�����¼���־��ʧ��\r\n");
    }

	/* ���������ź��� */
    xMutex = xSemaphoreCreateMutex();
	
	if(xMutex == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }  

  /* ����Test_Queue */
  MQTT_Data_Queue = xQueueCreate((UBaseType_t ) MQTT_QUEUE_LEN,/* ��Ϣ���еĳ��� */
                                 (UBaseType_t ) MQTT_QUEUE_SIZE);/* ��Ϣ�Ĵ�С */
  if(NULL != MQTT_Data_Queue)
    log_d("����MQTT_Data_Queue��Ϣ���гɹ�!\r\n");    

}

/*
*********************************************************************************************************
*	�� �� ��: vTaskStart
*	����˵��: �������񣬵ȴ����������¼���־������
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 4  
*********************************************************************************************************
*/
static void vTaskStart(void *pvParameters)
{
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS; /* ����ӳ�1000ms */
	
	/* 
	  ��ʼִ����������������ǰʹ�ܶ������Ź���
	  ����LSI��32��Ƶ�����溯��������Χ0-0xFFF���ֱ������Сֵ1ms�����ֵ4095ms
	  �������õ���4s�����4s��û��ι����ϵͳ��λ��
	*/
	bsp_InitIwdg(4000);
	
	/* ��ӡϵͳ����״̬������鿴ϵͳ�Ƿ�λ */
	App_Printf("=====================================================\r\n");
	App_Printf("=ϵͳ����ִ��\r\n");
	App_Printf("=====================================================\r\n");
	
    while(1)
    {        
		/* �ȴ������������¼���־ */
		uxBits = xEventGroupWaitBits(xCreatedEventGroup, /* �¼���־���� */
							         TASK_BIT_ALL,       /* �ȴ�TASK_BIT_ALL������ */
							         pdTRUE,             /* �˳�ǰTASK_BIT_ALL�������������TASK_BIT_ALL�������òű�ʾ���˳���*/
							         pdTRUE,             /* ����ΪpdTRUE��ʾ�ȴ�TASK_BIT_ALL��������*/
							         xTicksToWait); 	 /* �ȴ��ӳ�ʱ�� */
		
		if((uxBits & TASK_BIT_ALL) == TASK_BIT_ALL)
		{            
		    IWDG_Feed(); //ι��			
		}
	    else
		{
			/* ������ÿxTicksToWait����һ�� */
			/* ͨ������uxBits�򵥵Ŀ����ڴ˴�����Ǹ�������û�з������б�־ */
		}
    }
}



//LED������ 
void vTaskLed(void *pvParameters)
{
    uint8_t i = 100;
    while(1)
    {
        if(i == 250)
        {
            i = 100;
        }
        i+=20;
        bsp_SetTIMOutPWM(GPIOA, GPIO_Pin_8, TIM1, 1, 100, ((i) * 10000) /255);

        
//        LEDERROR = ! LEDERROR;        
        LED1=!LED1;  
        LED2=!LED2; 
        LED3=!LED3; 
        LED4=!LED4; 

//        RL3 = !RL3;
//                RL4 = !RL4;
        
		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_0);        
        
        vTaskDelay(300);
   
    }

}   

void vTaskDisplay(void *pvParameters)
{  
    while(1)
    {
        bsp_HC595Show(1,2,3);
        vTaskDelay(1000);
        bsp_HC595Show(4,5,6);
        vTaskDelay(1000);
        bsp_HC595Show(7,8,9);
        vTaskDelay(1000);
        bsp_HC595Show('a','b','c');
        vTaskDelay(1000);
        bsp_HC595Show('d','e','f');     
        vTaskDelay(1000);
        bsp_HC595Show('a',0,1);
        vTaskDelay(1000);
        bsp_HC595Show('d',3,4);   
        vTaskDelay(1000);
        bsp_HC595Show(1,0,1);  
        vTaskDelay(1000);

    }  
    //    while(1) 
    //    {       
    //        uint32_t val = 1;
    //        
    //        for(int i=0; i< 24; i++)
    //        {
    //            bsp_HC595RowOut(val);
    //            val <<= 1;
    //            vTaskDelay(500);
    //        }
    //    }

}


//motor to host ������
//void vTaskMortorToHost(void *pvParameters)
//{
//    uint8_t buf[16] = {0};
//    uint8_t readLen = 0;
//    while (1)
//    {       
//        if(bsp_Usart4_RecvOne(buf) == 1)
//        {  
//            
//           if(bsp_Usart4_RecvOne(buf+1) == 1)
//           {
//                 
//                 dbh("vTaskMortorToHost", (char *)buf, 16);
//                 
//                if(buf[1] == 0x03)//��ȡ״̬
//                {
//                   readLen=bsp_Usart4_RecvAtTime(buf + 2, 5, 20);                   
//                }
//                else if(buf[1] == 0x06)//��ȡִ��״̬
//                {
//                    readLen=bsp_Usart4_RecvAtTime(buf + 2, 6, 20);
//                }

//                if(readLen == 5 || readLen == 6) //���յ����ݲ�����
//                {
//                    send_to_host(CONTROLMOTOR,buf,readLen+2);
//                }
//           }
//        }
//        
//		/* �����¼���־����ʾ������������ */        
//		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);
//        
//        vTaskDelay(10);
//    }
//}



void vTaskKey(void *pvParameters)
{
    
	uint8_t ucKeyCode;
	uint8_t pcWriteBuffer[500];
    uint32_t id[3] = {0};

    uint16_t crc_value = 0;

    uint8_t cm4[] = { 0x02,0x7B,0x22,0x63,0x6D,0x64,0x22,0x3A,0x22,0x75,0x70,0x64,0x61,0x74,0x65,0x22,0x2C,0x22,0x76,0x61,0x6C,0x75,0x65,0x22,0x3A,0x7B,0x22,0x75,0x70,0x64,0x61,0x74,0x65,0x22,0x3A,0x22,0x41,0x37,0x22,0x7D,0x2C,0x22,0x64,0x61,0x74,0x61,0x22,0x3A,0x22,0x30,0x30,0x22,0x7D,0x03 };

    uint8_t open[8] = { 0x01,0x06,0x08,0x0C,0x00,0x02,0xCA,0x68 };
    uint8_t close[8] = { 0x01,0x06,0x08,0x0C,0x00,0x01,0x8A,0x69 };
    
    
    while(1)
    {
//		ucKeyCode = bsp_key_Scan(0);
        ucKeyCode = KEY_Scan(0);
      
		
		if (ucKeyCode != KEY_NONE)
		{
            //dbg("ucKeyCode = %d\r\n",ucKeyCode);
              
			switch (ucKeyCode)
			{
				/* K1������ ��ӡ����ִ����� */
				case KEY_SET_PRES:	             
					App_Printf("=================================================\r\n");
					App_Printf("������      ����״̬ ���ȼ�   ʣ��ջ �������\r\n");
					vTaskList((char *)&pcWriteBuffer);
					App_Printf("%s\r\n", pcWriteBuffer);
                    
					App_Printf("\r\n������       ���м���         ʹ����\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					App_Printf("%s\r\n", pcWriteBuffer);                    
					break;				
				/* K2�����£���ӡ���ڲ������� */
				case KEY_RR_PRES:
                
                    log_a("KEY_DOWN_K2\r\n");

			
					break;
				case KEY_LL_PRES:

                    log_i("KEY_DOWN_K3\r\n");                    

                    Get_ChipID(id);
                    App_Printf("mcu id = %x %x %x\r\n",id[0],id[1],id[2]); 
					break;
				case KEY_OK_PRES:    
       
                    log_w("KEY_DOWN_K4\r\n");
                    crc_value = CRC16_Modbus(cm4, 54);
                    log_v("hi = %02x, lo = %02x\r\n", crc_value>>8, crc_value & 0xff);

					break;                
				
				/* �����ļ�ֵ������ */
				default:   
				log_e("KEY_default\r\n");
					break;
			}
		}

        /* �����¼���־����ʾ������������ */
//		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_2);
		
		vTaskDelay(20);
	}   

}





void vTaskMsgPro(void *pvParameters)
{   
//    while(1)
//    {        
//        deal_Serial_Parse();    
//	    deal_rx_data();

//		/* �����¼���־����ʾ������������ */        
//		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_2);
//        vTaskDelay(10);
//    }


  


    uint8_t recv_buf[6] = {0x30,0x31,0x32,0x33,0x0d,0x0a};
//    uint8_t recv_buf[128] = {0};
    uint8_t open[8] = { 0x01,0x06,0x08,0x0C,0x00,0x02,0xCA,0x68 };



    uint8_t ch = 0;
    uint16_t len = 0;

    while(1)
    {
        len = comGetBuff(COM5,recv_buf,8,10);
        
        if(len == 8)
        {
           comSendBuf(COM5,recv_buf, len); 
        }
       


      /* �����¼���־����ʾ������������ */        
        xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_2);  
        vTaskDelay(10);        
    }    
    
}


void vTaskInfrared(void *pvParameters)
{
    int16_t code = 0;
    uint8_t dat[3] = {0};
//    CMD_RX_T cmd_rx;
    while(1)
    {
        code = bsp_infrared_scan();       


        if(code != ERR_INFRARED)
        {
            memset(dat,0x00,sizeof(dat));            
            dat[2] = code;            
            send_to_host(GETSENSOR,dat,3);

        }

		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_3);    
        
        vTaskDelay(1);
    }
}

void vTaskRs485(void *pvParameters)
{
    uint8_t recv_buf[32] = {0};
    uint16_t readlen = 0;
    
    while (1)
    {     
        memset(recv_buf,0x00,sizeof(recv_buf));        

       #ifndef RS485TEST
        readlen = bsp_RS485_Receive_Data(recv_buf,8);
        #else
        readlen = RS485_Recv(COM1,recv_buf,8);
        #endif  


        
        if(readlen == 8)
        {
//            BEEP = ~BEEP;
            dbh("RS485 recv", (char *)recv_buf, readlen);


//            send_to_host(DOOR_B,recv_buf,readlen);
        }

		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_6);
        
        vTaskDelay(20);
    }
    
}


void vTaskReader(void *pvParameters)
{ 
    uint32_t CardID = 0;
    uint8_t dat[4] = {0};
    while(1)
    {
        CardID = bsp_WeiGenScanf();

        if(CardID != 0)
        {
            memset(dat,0x00,sizeof(dat));            
            
			dat[0] = CardID>>24;
			dat[1] = CardID>>16;
			dat[2] = CardID>>8;
			dat[3] = CardID&0XFF;    
            
            send_to_host(WGREADER,dat,4);
        }  


		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_4);        
        
        vTaskDelay(20);
        
    }

}   


void vTaskQR(void *pvParameters)
{ 
    uint8_t recv_buf[128] = {0};
    uint8_t dat[128] = {0};
    uint16_t len = 0;

    mqtt_thread();

    while(1)
    {

        
//       memset(recv_buf,0x00,sizeof(recv_buf));  
//       memset(dat,0x00,sizeof(dat));  
//       len = bsp_Usart3_RecvAtTime(recv_buf,18,100); 
//       if(len == 18)
//       {
//            asc2bcd(dat, recv_buf, len, 0);         
//            send_to_host(QRREADER,dat,9);
//       }

		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_5);        
        
        vTaskDelay(500);        
    }

}   


//��ӡ��ַ����Ϣ
void vTaskLwip(void *pvParameters)
{
	while(1)
	{ 

#if LWIP_DHCP									        //������DHCP��ʱ��     
		if(lwipdev.dhcpstatus != 0) 					//����DHCP
		{
			show_address(lwipdev.dhcpstatus );			//��ʾ��ַ��Ϣ
			vTaskSuspend(xHandleTaskLwip); 		//��ʾ���ַ��Ϣ�������������
		}
#else
		show_address(0); 						        //��ʾ��̬��ַ
		vTaskSuspend(xHandleTaskLwip); 			//��ʾ���ַ��Ϣ�������������
#endif //LWIP_DHCP
		vTaskDelay(500);      							//��ʱ500ms
	}
}




/*
*********************************************************************************************************
*	�� �� ��: App_Printf
*	����˵��: �̰߳�ȫ��printf��ʽ		  			  
*	��    ��: ͬprintf�Ĳ�����
*             ��C�У����޷��г����ݺ���������ʵ�ε����ͺ���Ŀʱ,������ʡ�Ժ�ָ��������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void  App_Printf(char *format, ...)
{
    char  buf_str[200 + 1];
    va_list   v_args;


    va_start(v_args, format);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) format,
                                  v_args);
    va_end(v_args);

	/* �����ź��� */
	xSemaphoreTake(xMutex, portMAX_DELAY);

    printf("%s", buf_str);

   	xSemaphoreGive(xMutex);
}



static void show_address(u8 mode)
{
	u8 buf[30];
	if(mode==2)
	{
		sprintf((char*)buf,"MAC    :%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);//��ӡMAC��ַ
		App_Printf((char*)buf); 
		sprintf((char*)buf,"DHCP IP:%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//��ӡ��̬IP��ַ
		App_Printf((char*)buf); 
		sprintf((char*)buf,"DHCP GW:%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//��ӡ���ص�ַ
		App_Printf((char*)buf); 
		sprintf((char*)buf,"DHCP IP:%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//��ӡ���������ַ
		App_Printf((char*)buf); 
	}
	else 
	{
		sprintf((char*)buf,"MAC      :%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);//��ӡMAC��ַ
		App_Printf((char*)buf); 
		sprintf((char*)buf,"Static IP:%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//��ӡ��̬IP��ַ
		App_Printf((char*)buf); 
		sprintf((char*)buf,"Static GW:%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//��ӡ���ص�ַ
		App_Printf((char*)buf); 
		sprintf((char*)buf,"Static IP:%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//��ӡ���������ַ
		App_Printf((char*)buf); 
	}	
}

static void EasyLogInit(void)
{
    /* initialize EasyLogger */
     elog_init();
     /* set EasyLogger log format */
     elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
     elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_TIME);
     elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_TIME);

     
     /* start EasyLogger */
     elog_start();  
}




