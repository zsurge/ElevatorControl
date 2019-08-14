/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : comm.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年6月18日
  最近修改   :
  功能描述   : 解析串口指令
  函数列表   :
  修改历史   :
  1.日    期   : 2019年6月18日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "comm.h"
//#include "bsp_usart4.h"
//#include "bsp_usart3.h"
#include "tool.h"
#include "bsp_led.h"
#include "malloc.h"
#include "ini.h"
#include "ymodem.h"
//#include "bsp_rs485.h"

#include "bsp_uart_fifo.h"


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_RXD_BUF_LEN        			128
#define MAX_TXD_BUF_LEN					128      
        
/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
typedef struct
{
    uint8_t RxdStatus;                 //接收状态
    uint8_t RxCRCHi;                   //校验值高8位
    uint8_t RxCRCLo;                   //校验值低8位
    uint8_t RxdFrameStatus;            //接收包状态
    uint8_t NowLen;                    //接收字节指针
    uint8_t RxdTotalLen;               //接收包数据长度
    uint8_t RxdBuf[MAX_RXD_BUF_LEN];   //接收包数据缓存
}RECVHOST_T;

static RECVHOST_T gRecvHost;

static uint16_t crc_value = 0;


static COMM_ERR parseJSON(uint8_t *text,CMD_RX_T *cmd_rx); //私有函数
static uint8_t  packetJSON(CMD_TX_T *cmd_tx,uint8_t *command_data);



/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

    
void init_serial_boot(void)
{
//    gRecvHost.RxdStatus = 0;
//    gRecvHost.RxCRCHi = 0;
//	gRecvHost.RxCRCLo = 0;
//    gRecvHost.RxdFrameStatus = SPACE;

    memset(&gRecvHost,0x00,sizeof(gRecvHost));
}

/*****************************************************************************
 函 数 名  : deal_Serial_Parse
 功能描述  :     将收到的数据解析成数据包；
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年6月18日
    作    者   : 张舵
    修改内容   : 新生成函数

*****************************************************************************/
// 7E 01 A1 00 AA 74 
// 7E 01 A1 AA 25 48 00 AA 74 7E 01 A1 00 AA 74


void deal_Serial_Parse(void)
{
    uint8_t ch = 0; 
    
    while(1)
    {  
        #if CMD_SERIAL_PORT == 0x01
        if(comGetChar(COM1, &ch) != 1)  //读取串口数据
        {
            delay_ms(10);
            
            return;
        }
        #else
        if(comGetChar(COM2, &ch) != 1)  //读取串口数据
        {
            delay_ms(10);
            
            return;
        }
        #endif
        
        switch (gRecvHost.RxdStatus)
        { /*接收数据状态*/                
            case 0:
                if(STX == ch) /*接收包头*/
                {
                    gRecvHost.RxdBuf[0] = ch;
                    gRecvHost.NowLen = 1;
                    gRecvHost.RxdStatus = 20;
                }
                break;              
            case 20:      /* 接收整个数据包 */
                if (ch == ETX) 
                {
                    gRecvHost.RxdStatus = 21;
                }
                gRecvHost.RxdBuf[gRecvHost.NowLen++] = ch; //ETX

                break;
            case 21:
                crc_value = CRC16_Modbus((uint8_t *)gRecvHost.RxdBuf, gRecvHost.NowLen);                
                gRecvHost.RxCRCHi = crc_value>>8;
                gRecvHost.RxCRCLo = crc_value & 0xFF;                
                gRecvHost.RxdStatus = 22;
                gRecvHost.RxdBuf[gRecvHost.NowLen++] = ch; //CRCHI
                
                break;
           case 22:
                gRecvHost.RxdBuf[gRecvHost.NowLen++] = ch; //CRCLO
                
                if(gRecvHost.RxdBuf[gRecvHost.NowLen-2] == gRecvHost.RxCRCHi && \
                    gRecvHost.RxdBuf[gRecvHost.NowLen-1] == gRecvHost.RxCRCLo )
                {    
                    gRecvHost.RxdTotalLen = gRecvHost.NowLen;
                    gRecvHost.RxdFrameStatus = FINISH;                    
                    gRecvHost.RxdStatus = 0;
                    printf("Deal Serial Parse finish\r\n");
                }
                else
                {
                    gRecvHost.RxdFrameStatus = FINISH;
                    gRecvHost.RxdBuf[gRecvHost.NowLen++] = ch;
                    gRecvHost.RxdStatus = 20;
                }      
                break;
               
            default:                
                if (gRecvHost.RxdFrameStatus == SPACE) 
                {
                    gRecvHost.RxdFrameStatus = FINISH;
                    gRecvHost.RxdStatus = 0;
                }
            break;
         }
    }
}



/*****************************************************************************
 函 数 名  : deal_rx_data
 功能描述  : 解析收到的完整数据包，并响应数据包；
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年6月18日
    作    者   : 张舵
    修改内容   : 新生成函数

*****************************************************************************/
void deal_rx_data(void)
{
    uint8_t bccHi = 0;       
    uint8_t bccLo = 0; 


    uint8_t json_buf[JSON_PACK_MAX] = {0};
    CMD_RX_T cmd_rx;
    
    
    if (FINISH == gRecvHost.RxdFrameStatus)
    {        
        if(gRecvHost.RxdBuf[0] == STX && gRecvHost.RxdBuf[gRecvHost.RxdTotalLen -3] == ETX)                                   //解析02数据包
        {            
            //计算校验值
            bccHi =  crc_value >> 8;
            bccLo = crc_value & 0xff;
            
            if ((bccHi == gRecvHost.RxdBuf[gRecvHost.RxdTotalLen -2]) && \
                (bccLo == gRecvHost.RxdBuf[gRecvHost.RxdTotalLen -1]))
            {
				//转存JSON数据包
                memcpy(json_buf,(const char*)gRecvHost.RxdBuf+1,gRecvHost.RxdTotalLen-4);
                printf("recv json data = : %s\r\n",json_buf);

                //解析JSON数据包              
                if(parseJSON(json_buf,&cmd_rx) == COMM_SUCCESS)
                {
                    //向外设发送指令而不是主机，目前做为测试用
                    dbg("parseJSON success!\r\n");
                    send_to_device(&cmd_rx);
                }
                else
                {
                    //指令解析失败，向上位机发送解析失败的状态，要求重发
                }

                init_serial_boot();   


            }
            else
            {
                init_serial_boot();
            }
        }
        else
        {
            init_serial_boot();
        }
    }

}



uint8_t send_to_host(uint8_t cmd,uint8_t *buf,uint8_t len)
{
    uint8_t i = 0;
    uint8_t json_len = 0;
    uint8_t TxdBuf[MAX_TXD_BUF_LEN]={0};
    uint8_t tmpBuf[MAX_TXD_BUF_LEN] = {0};
    uint16_t iCRC = 0;
    CMD_TX_T cmd_tx;


    memset(tmpBuf,0x00,sizeof(tmpBuf));
    memset(TxdBuf,0x00,sizeof(TxdBuf));
    memset(&cmd_tx,0x00,sizeof(cmd_tx));

    i = 3;
    TxdBuf[0] = STX;
    cmd_tx.cmd = cmd;
    cmd_tx.code = 0;
    
    bcd2asc(cmd_tx.data, buf, len*2, 0);        
    json_len = packetJSON(&cmd_tx,tmpBuf);  
    if(json_len == 0)
    {
        printf("<<<<<<<<packet json error>>>>>>>>\r\n");
        return 255;
    }
    else
    {
        i += json_len;
    }
    memcpy(TxdBuf+3,tmpBuf,i-3); 
    TxdBuf[i++] = ETX;   

    TxdBuf[1] = i>>8; //high
    TxdBuf[2] = i&0xFF; //low
            
    iCRC = CRC16_Modbus(TxdBuf, i);  
    TxdBuf[i++] = iCRC >> 8;
    TxdBuf[i++] = iCRC & 0xff;  

//    dbh("send_to_host",(char *)TxdBuf,i);

#if CMD_SERIAL_PORT == 0x01
//    bsp_Usart1_SendData(TxdBuf,i);
    comSendBuf(COM3,TxdBuf,i);

#else
//    bsp_Usart2_SendData(TxdBuf,i);
    comSendBuf(COM2,TxdBuf,i);
#endif

    return 0;
}


static COMM_ERR parseJSON(uint8_t *text,CMD_RX_T *cmd_rx)
{
    cJSON  *root = NULL;			                    // root    
    cJSON  *cmd = NULL;			                        // cmd     
    COMM_ERR result = COMM_SUCCESS;
    
    uint8_t bcd_cmd[2] = {0};
    uint8_t bcd_dat[16] = {0};
    uint8_t *tmpCmd = NULL;
    uint8_t *tmpdat = NULL;
    uint8_t asc_len = 0;  

    //{"cmd":"sensor","value":{"sensor":"A1"}}    


    
    root = cJSON_Parse((const char*)text);

    if (root == NULL)                 // 如果转化错误，则报错退出
    {
        printf("1.Error before: [%s]\n", cJSON_GetErrorPtr());
        return COMM_ERR_PARSE;
    }

    //获取KEY,指令描述
    cmd_rx->cmd_desc = (uint8_t *)cJSON_GetObjectItem(root,"cmd")->valuestring;  
    if(cmd_rx->cmd_desc == NULL)
    {
        printf("2.Error before: [%s]\n", cJSON_GetErrorPtr());
        return COMM_ERR_GETITEM;
    }
     printf("cmd_desc = %s\r\n",cmd_rx->cmd_desc);

    //获取指令字
    cmd = cJSON_GetObjectItem(root,"value"); 
    if(cmd == NULL)
    {
        printf("3.Error before: [%s]\n", cJSON_GetErrorPtr());
        return COMM_ERR_GETITEM;
    }    

    tmpCmd = (uint8_t *)cJSON_GetObjectItem(cmd,(const char*)cmd_rx->cmd_desc)->valuestring; 
    if(tmpCmd == NULL)
    {
        printf("3.Error before: [%s]\n", cJSON_GetErrorPtr());
        return COMM_ERR_GETITEM;
    }      
    printf("cmd_value = %s, length = %d\r\n",tmpCmd,strlen((const char*)tmpCmd));

    asc2bcd(bcd_cmd, tmpCmd, strlen((const char*)tmpCmd), 0);

    //目前指令只有1byte 所以直接赋值
    cmd_rx->cmd = bcd_cmd[0];
    printf("cmd_rx.cmd =  %02x\r\n",cmd_rx->cmd);

    memset(tmpdat,0x00,sizeof(tmpdat));
    tmpdat = (uint8_t *)cJSON_GetObjectItem(root,"data")->valuestring;  
    dbh("tmpdat", (char *)tmpdat, 16);



    asc_len = strlen((const char*)tmpdat);

    //若是有数据，则转换;无数据则不处理
    if(asc_len > 0)
    {
        if(asc_len % 2 != 0)
        {
            asc_len += 1;
        }
        asc2bcd(bcd_dat,tmpdat,asc_len,0);
        memcpy(cmd_rx->cmd_data,bcd_dat,asc_len/2);

        dbg("cmd_rx->cmd_data = %s\r\n",cmd_rx->cmd_data);
    }
    

    cJSON_Delete(root);

    my_free(cmd);
    my_free(tmpCmd);
    my_free(tmpdat);
    
    return result;

}



static uint8_t  packetJSON(CMD_TX_T *cmd_tx,uint8_t *command_data)
{
    char *TxdBuf;
    cJSON *root; // cJSON指针
    uint8_t len = 0;//返回json的长度

    uint8_t tmp_code = cmd_tx->code;
    uint8_t tmp_data[64] = {0}; 
    
    memset(tmp_data,0x00,sizeof(tmp_data));
    root=cJSON_CreateObject(); // 创建root对象，返回值为cJSON指针

    if (root == NULL)                 // 如果转化错误，则报错退出
    {
        printf("4.Error before: [%s]\n", cJSON_GetErrorPtr());
        return 0;
    }

    //这里若是不中转则，JSON打包的数据会变，原因未知  2019.07.12 surge
    memcpy(tmp_data,cmd_tx->data,strlen((const char*)cmd_tx->data));
    
    sprintf(TxdBuf,"%02x",cmd_tx->cmd);
    
    cJSON_AddStringToObject(root,"cmd",TxdBuf);
    cJSON_AddNumberToObject(root,"code",tmp_code);
    cJSON_AddStringToObject(root,"data",(const char*)tmp_data);
    
    memset(TxdBuf,0x00,sizeof(TxdBuf));
    TxdBuf = cJSON_PrintUnformatted(root); 

    if(TxdBuf == NULL)
    {
        printf("5.Error before: [%s]\n", cJSON_GetErrorPtr());
        return 0;
    }

    strcpy((char *)command_data,TxdBuf);

    printf("command_data = %s\r\n",TxdBuf);

    len = strlen((const char*)TxdBuf);

    cJSON_Delete(root);

    my_free(TxdBuf);

    return len;
}


void send_to_device(CMD_RX_T *cmd_rx)
{
    uint8_t i = 0;
    uint8_t TxdBuf[MAX_TXD_BUF_LEN]={0};
    uint8_t tmpBuf[MAX_TXD_BUF_LEN] = {0};
    uint16_t iCRC = 0;
    CMD_TX_T cmd_tx;

    dbg("cmd_rx.cmd = %02x\r\n",cmd_rx->cmd);
    
    memset(&cmd_tx,0x00,sizeof(cmd_tx));
    
    switch (cmd_rx->cmd)
    {
        case GETSENSOR://获取红外状态，目前只有两组红外
            i = 3;
            TxdBuf[0] = STX;            
            cmd_tx.cmd = GETSENSOR;
            cmd_tx.code = 0x00;
            bsp_GetSensorStatus(cmd_tx.data);
            dbg("2cmd_tx.data = %s\r\n",cmd_tx.data);         
            i += packetJSON(&cmd_tx,tmpBuf);            
            memcpy(TxdBuf+3,tmpBuf,i-3); 
            TxdBuf[i++] = ETX;  
            
            TxdBuf[1] = i>>8; //high
            TxdBuf[2] = i&0xFF; //low
            
            iCRC = CRC16_Modbus(TxdBuf, i);  
            
            TxdBuf[i++] = iCRC >> 8;
            TxdBuf[i++] = iCRC & 0xff;            
            break;
        case SETLED: //设置LED灯
            i = 3;
            TxdBuf[0] = STX;
            cmd_tx.cmd = SETLED;
            cmd_tx.code = 0x00;
            dbg("cmd_rx->cmd_data = %s,len = %d\r\n",cmd_rx->cmd_data,strlen((const char*)cmd_rx->cmd_data));
            bsp_Ex_SetLed(cmd_rx->cmd_data); 
            strcpy((char *)cmd_tx.data,"00");            
            i += packetJSON(&cmd_tx,tmpBuf);            
            memcpy(TxdBuf+3,tmpBuf,i-3); 
            TxdBuf[i++] = ETX;     

            TxdBuf[1] = i>>8; //high
            TxdBuf[2] = i&0xFF; //low
            
            iCRC = CRC16_Modbus(TxdBuf, i);  
            TxdBuf[i++] = iCRC >> 8;
            TxdBuf[i++] = iCRC & 0xff;   
            break;                        
        case GETDEVICESAT://获取设备状态

            break; 
        case GETVER:
            i = 3;
            TxdBuf[0] = STX;
            cmd_tx.cmd = GETVER;
            cmd_tx.code = 0x00;
            dbg("cmd_rx->cmd_data = %s,len = %d\r\n",cmd_rx->cmd_data,strlen((const char*)cmd_rx->cmd_data));
            strcpy((char *)cmd_tx.data,"V1.0.1");            
            i += packetJSON(&cmd_tx,tmpBuf); 
            memcpy(TxdBuf+3,tmpBuf,i-3); 
            TxdBuf[i++] = ETX;     

            TxdBuf[1] = i>>8; //high
            TxdBuf[2] = i&0xFF; //low
            
            iCRC = CRC16_Modbus(TxdBuf, i);  
            TxdBuf[i++] = iCRC >> 8;
            TxdBuf[i++] = iCRC & 0xff;   
            break; 
            
        case HEARTBEAT:

            break;
        case UPGRADE:
            SystemReset();
            break;   

        case CONTROLMOTOR:       
             //向电机发送控制指令
            dbg("<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>\r\n");
            dbh("CONTROL MOTOR", (char *)cmd_rx->cmd_data, 8);
//            bsp_Usart4_SendData(cmd_rx->cmd_data,8);//发给主机
            comSendBuf(COM1,cmd_rx->cmd_data,8);
            return;
            
        case DOOR_B:
            dbh("CONTROL MOTOR door B", (char *)cmd_rx->cmd_data, 8);
//            bsp_RS485_Send_Data(cmd_rx->cmd_data,8);//发给B门
            return;//这里不需要向上位机上送，在另外一个任务中才上送

        default:
            init_serial_boot(); 
            return;
    }

//    dbh("send_to_device",(char *)TxdBuf,i);
#if CMD_SERIAL_PORT == 0x01
//    bsp_Usart1_SendData(TxdBuf,i);
    comSendBuf(COM1,TxdBuf,i);

#else
//    bsp_Usart2_SendData(TxdBuf,i);
    comSendBuf(COM2,TxdBuf,i);

#endif 
}



