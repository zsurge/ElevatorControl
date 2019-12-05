/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : main.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年7月9日
  最近修改   :
  功能描述   : 主程序模块
  函数列表   :
  修改历史   :
  1.日    期   : 2019年7月9日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/


#include "INCLUDES.h"

#include "client.h"
#include "tcp_server_demo.h"

#include "mqtt_app.h"

#define LOG_TAG    "main"
#include "elog.h"


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
//任务优先级   

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


//任务堆栈大小    
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


#define  MQTT_QUEUE_LEN    4   /* 队列的长度，最大可包含多少个消息 */
#define  MQTT_QUEUE_SIZE   4   /* 队列中每个消息大小（字节） */



//事件标志
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
 * 模块级变量                                   *
 *----------------------------------------------*/
//任务句柄
static TaskHandle_t xHandleTaskLed = NULL;      //LED灯
static TaskHandle_t xHandleTaskMotor = NULL;    //电机控制
static TaskHandle_t xHandleTaskCmd = NULL;      //android通讯
static TaskHandle_t xHandleTaskInfrared = NULL; //红外感映
static TaskHandle_t xHandleTaskReader = NULL;   //韦根读卡器
static TaskHandle_t xHandleTaskQr = NULL;       //二维码读头
static TaskHandle_t xHandleTaskRs485 = NULL;
static TaskHandle_t xHandleTaskStart = NULL;    //看门狗

static TaskHandle_t xHandleTaskKey = NULL;    //按键
static TaskHandle_t xHandleTaskLwip = NULL;    //按键
static TaskHandle_t xHandleTaskApp = NULL;    ////创建用户任务




static SemaphoreHandle_t  xMutex = NULL;
static EventGroupHandle_t xCreatedEventGroup = NULL;
QueueHandle_t MQTT_Data_Queue =NULL;


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

//任务函数
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
    //硬件初始化
    bsp_Init();  

    EasyLogInit();    

	mymem_init(SRAMIN);								//初始化内部内存池
	mymem_init(SRAMEX);								//初始化外部内存池
	mymem_init(SRAMCCM);	  					    //初始化CCM内存池

	while(lwip_comm_init() != 0) //lwip初始化
	{
        log_d("lwip init error!\r\n");
		delay_ms(1200);
	}

    log_d("lwip init success!\r\n");

	/* 创建任务通信机制 */
	AppObjCreate(); 

    


    
	//创建开始任务
    xTaskCreate((TaskFunction_t )vTaskAppCreate,            //任务函数
                (const char*    )"AppCreate",          //任务名称
                (uint16_t       )APP_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )APP_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&xHandleTaskApp);   //任务句柄          
                
    /* 启动调度，开始执行任务 */
    vTaskStartScheduler();
    
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void vTaskAppCreate (void *pvParameters)
{

#if LWIP_DHCP
	lwip_comm_dhcp_creat(); 							//创建DHCP任务
#endif

//    tcp_server_init(); //tcp server demo

//    mqtt_thread_init();

//    client_init();

    taskENTER_CRITICAL();           //进入临界区

    //创建LED任务
    xTaskCreate((TaskFunction_t )vTaskLed,         
                (const char*    )"vTaskLed",       
                (uint16_t       )LED_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )LED_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskLed);   

    //创建电机信息返回任务
//    xTaskCreate((TaskFunction_t )vTaskMortorToHost,     
//                (const char*    )"vTMTHost",   
//                (uint16_t       )MOTOR_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )MOTOR_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskMotor);                       

    //键盘测试
    xTaskCreate((TaskFunction_t )vTaskKey,     
                (const char*    )"vTaskKey",   
                (uint16_t       )KEY_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )KEY_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskKey);                 
                

    //跟android通讯串口数据解析
//    xTaskCreate((TaskFunction_t )vTaskMsgPro,     
//                (const char*    )"cmd",   
//                (uint16_t       )CMD_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )CMD_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskCmd);      

    //红外传感器状态上送
//    xTaskCreate((TaskFunction_t )vTaskInfrared,     
//                (const char*    )"vTaskInfrared",   
//                (uint16_t       )INFRARED_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )INFRARED_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskInfrared);    


    //485任务
//    xTaskCreate((TaskFunction_t )vTaskRs485,     
//                (const char*    )"vTaskRs485",   
//                (uint16_t       )RS485_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )RS485_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskRs485);  

    //韦根读卡器
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

    //二维码扫码模块    
//    xTaskCreate((TaskFunction_t )vTaskDisplay, 
    xTaskCreate((TaskFunction_t )vTaskQR,     
                (const char*    )"vTaskQR",   
                (uint16_t       )QR_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )QR_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskQr);                  

    //看门狗
//	xTaskCreate((TaskFunction_t )vTaskStart,     		/* 任务函数  */
//                (const char*    )"vTaskStart",   		/* 任务名    */
//                (uint16_t       )START_STK_SIZE,        /* 任务栈大小，单位word，也就是4字节 */
//                (void*          )NULL,           		/* 任务参数  */
//                (UBaseType_t    )START_TASK_PRIO,       /* 任务优先级*/
//                (TaskHandle_t*  )&xHandleTaskStart );   /* 任务句柄  */    

	vTaskDelete(xHandleTaskApp); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区

}


/*
*********************************************************************************************************
*	函 数 名: AppObjCreate
*	功能说明: 创建任务通信机制
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppObjCreate (void)
{
	/* 创建事件标志组 */
	xCreatedEventGroup = xEventGroupCreate();
	
	if(xCreatedEventGroup == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
        App_Printf("创建事件标志组失败\r\n");
    }

	/* 创建互斥信号量 */
    xMutex = xSemaphoreCreateMutex();
	
	if(xMutex == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }  

  /* 创建Test_Queue */
  MQTT_Data_Queue = xQueueCreate((UBaseType_t ) MQTT_QUEUE_LEN,/* 消息队列的长度 */
                                 (UBaseType_t ) MQTT_QUEUE_SIZE);/* 消息的大小 */
  if(NULL != MQTT_Data_Queue)
    log_d("创建MQTT_Data_Queue消息队列成功!\r\n");    

}

/*
*********************************************************************************************************
*	函 数 名: vTaskStart
*	功能说明: 启动任务，等待所有任务发事件标志过来。
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 4  
*********************************************************************************************************
*/
static void vTaskStart(void *pvParameters)
{
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS; /* 最大延迟1000ms */
	
	/* 
	  开始执行启动任务主函数前使能独立看门狗。
	  设置LSI是32分频，下面函数参数范围0-0xFFF，分别代表最小值1ms和最大值4095ms
	  下面设置的是4s，如果4s内没有喂狗，系统复位。
	*/
	bsp_InitIwdg(4000);
	
	/* 打印系统开机状态，方便查看系统是否复位 */
	App_Printf("=====================================================\r\n");
	App_Printf("=系统开机执行\r\n");
	App_Printf("=====================================================\r\n");
	
    while(1)
    {        
		/* 等待所有任务发来事件标志 */
		uxBits = xEventGroupWaitBits(xCreatedEventGroup, /* 事件标志组句柄 */
							         TASK_BIT_ALL,       /* 等待TASK_BIT_ALL被设置 */
							         pdTRUE,             /* 退出前TASK_BIT_ALL被清除，这里是TASK_BIT_ALL都被设置才表示“退出”*/
							         pdTRUE,             /* 设置为pdTRUE表示等待TASK_BIT_ALL都被设置*/
							         xTicksToWait); 	 /* 等待延迟时间 */
		
		if((uxBits & TASK_BIT_ALL) == TASK_BIT_ALL)
		{            
		    IWDG_Feed(); //喂狗			
		}
	    else
		{
			/* 基本是每xTicksToWait进来一次 */
			/* 通过变量uxBits简单的可以在此处检测那个任务长期没有发来运行标志 */
		}
    }
}



//LED任务函数 
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
        
		/* 发送事件标志，表示任务正常运行 */        
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


//motor to host 任务函数
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
//                if(buf[1] == 0x03)//读取状态
//                {
//                   readLen=bsp_Usart4_RecvAtTime(buf + 2, 5, 20);                   
//                }
//                else if(buf[1] == 0x06)//读取执行状态
//                {
//                    readLen=bsp_Usart4_RecvAtTime(buf + 2, 6, 20);
//                }

//                if(readLen == 5 || readLen == 6) //接收到数据才上送
//                {
//                    send_to_host(CONTROLMOTOR,buf,readLen+2);
//                }
//           }
//        }
//        
//		/* 发送事件标志，表示任务正常运行 */        
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
				/* K1键按下 打印任务执行情况 */
				case KEY_SET_PRES:	             
					App_Printf("=================================================\r\n");
					App_Printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
					vTaskList((char *)&pcWriteBuffer);
					App_Printf("%s\r\n", pcWriteBuffer);
                    
					App_Printf("\r\n任务名       运行计数         使用率\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					App_Printf("%s\r\n", pcWriteBuffer);                    
					break;				
				/* K2键按下，打印串口操作命令 */
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
				
				/* 其他的键值不处理 */
				default:   
				log_e("KEY_default\r\n");
					break;
			}
		}

        /* 发送事件标志，表示任务正常运行 */
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

//		/* 发送事件标志，表示任务正常运行 */        
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
       


      /* 发送事件标志，表示任务正常运行 */        
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

		/* 发送事件标志，表示任务正常运行 */        
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

		/* 发送事件标志，表示任务正常运行 */        
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


		/* 发送事件标志，表示任务正常运行 */        
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

		/* 发送事件标志，表示任务正常运行 */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_5);        
        
        vTaskDelay(500);        
    }

}   


//打印地址等信息
void vTaskLwip(void *pvParameters)
{
	while(1)
	{ 

#if LWIP_DHCP									        //当开启DHCP的时候     
		if(lwipdev.dhcpstatus != 0) 					//开启DHCP
		{
			show_address(lwipdev.dhcpstatus );			//显示地址信息
			vTaskSuspend(xHandleTaskLwip); 		//显示完地址信息后挂起自身任务
		}
#else
		show_address(0); 						        //显示静态地址
		vTaskSuspend(xHandleTaskLwip); 			//显示完地址信息后挂起自身任务
#endif //LWIP_DHCP
		vTaskDelay(500);      							//延时500ms
	}
}




/*
*********************************************************************************************************
*	函 数 名: App_Printf
*	功能说明: 线程安全的printf方式		  			  
*	形    参: 同printf的参数。
*             在C中，当无法列出传递函数的所有实参的类型和数目时,可以用省略号指定参数表
*	返 回 值: 无
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

	/* 互斥信号量 */
	xSemaphoreTake(xMutex, portMAX_DELAY);

    printf("%s", buf_str);

   	xSemaphoreGive(xMutex);
}



static void show_address(u8 mode)
{
	u8 buf[30];
	if(mode==2)
	{
		sprintf((char*)buf,"MAC    :%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);//打印MAC地址
		App_Printf((char*)buf); 
		sprintf((char*)buf,"DHCP IP:%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//打印动态IP地址
		App_Printf((char*)buf); 
		sprintf((char*)buf,"DHCP GW:%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//打印网关地址
		App_Printf((char*)buf); 
		sprintf((char*)buf,"DHCP IP:%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//打印子网掩码地址
		App_Printf((char*)buf); 
	}
	else 
	{
		sprintf((char*)buf,"MAC      :%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);//打印MAC地址
		App_Printf((char*)buf); 
		sprintf((char*)buf,"Static IP:%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//打印动态IP地址
		App_Printf((char*)buf); 
		sprintf((char*)buf,"Static GW:%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//打印网关地址
		App_Printf((char*)buf); 
		sprintf((char*)buf,"Static IP:%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//打印子网掩码地址
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




