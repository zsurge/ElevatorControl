/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : comm.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��6��18��
  ����޸�   :
  ��������   : ��������ָ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��6��18��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
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
 * �궨��                                       *
 *----------------------------------------------*/
#define MAX_RXD_BUF_LEN        			128
#define MAX_TXD_BUF_LEN					128      
        
/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
typedef struct
{
    uint8_t RxdStatus;                 //����״̬
    uint8_t RxCRCHi;                   //У��ֵ��8λ
    uint8_t RxCRCLo;                   //У��ֵ��8λ
    uint8_t RxdFrameStatus;            //���հ�״̬
    uint8_t NowLen;                    //�����ֽ�ָ��
    uint8_t RxdTotalLen;               //���հ����ݳ���
    uint8_t RxdBuf[MAX_RXD_BUF_LEN];   //���հ����ݻ���
}RECVHOST_T;

static RECVHOST_T gRecvHost;

static uint16_t crc_value = 0;


static COMM_ERR parseJSON(uint8_t *text,CMD_RX_T *cmd_rx); //˽�к���
static uint8_t  packetJSON(CMD_TX_T *cmd_tx,uint8_t *command_data);



/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
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
 �� �� ��  : deal_Serial_Parse
 ��������  :     ���յ������ݽ��������ݰ���
 �������  : void
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��6��18��
    ��    ��   : �Ŷ�
    �޸�����   : �����ɺ���

*****************************************************************************/
// 7E 01 A1 00 AA 74 
// 7E 01 A1 AA 25 48 00 AA 74 7E 01 A1 00 AA 74


void deal_Serial_Parse(void)
{
    uint8_t ch = 0; 
    
    while(1)
    {  
        #if CMD_SERIAL_PORT == 0x01
        if(comGetChar(COM1, &ch) != 1)  //��ȡ��������
        {
            delay_ms(10);
            
            return;
        }
        #else
        if(comGetChar(COM2, &ch) != 1)  //��ȡ��������
        {
            delay_ms(10);
            
            return;
        }
        #endif
        
        switch (gRecvHost.RxdStatus)
        { /*��������״̬*/                
            case 0:
                if(STX == ch) /*���հ�ͷ*/
                {
                    gRecvHost.RxdBuf[0] = ch;
                    gRecvHost.NowLen = 1;
                    gRecvHost.RxdStatus = 20;
                }
                break;              
            case 20:      /* �����������ݰ� */
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
 �� �� ��  : deal_rx_data
 ��������  : �����յ����������ݰ�������Ӧ���ݰ���
 �������  : void
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��6��18��
    ��    ��   : �Ŷ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void deal_rx_data(void)
{
    uint8_t bccHi = 0;       
    uint8_t bccLo = 0; 


    uint8_t json_buf[JSON_PACK_MAX] = {0};
    CMD_RX_T cmd_rx;
    
    
    if (FINISH == gRecvHost.RxdFrameStatus)
    {        
        if(gRecvHost.RxdBuf[0] == STX && gRecvHost.RxdBuf[gRecvHost.RxdTotalLen -3] == ETX)                                   //����02���ݰ�
        {            
            //����У��ֵ
            bccHi =  crc_value >> 8;
            bccLo = crc_value & 0xff;
            
            if ((bccHi == gRecvHost.RxdBuf[gRecvHost.RxdTotalLen -2]) && \
                (bccLo == gRecvHost.RxdBuf[gRecvHost.RxdTotalLen -1]))
            {
				//ת��JSON���ݰ�
                memcpy(json_buf,(const char*)gRecvHost.RxdBuf+1,gRecvHost.RxdTotalLen-4);
                printf("recv json data = : %s\r\n",json_buf);

                //����JSON���ݰ�              
                if(parseJSON(json_buf,&cmd_rx) == COMM_SUCCESS)
                {
                    //�����跢��ָ�������������Ŀǰ��Ϊ������
                    DBG("parseJSON success!\r\n");
                    send_to_device(&cmd_rx);
                }
                else
                {
                    //ָ�����ʧ�ܣ�����λ�����ͽ���ʧ�ܵ�״̬��Ҫ���ط�
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

    if (root == NULL)                 // ���ת�������򱨴��˳�
    {
        printf("1.Error before: [%s]\n", cJSON_GetErrorPtr());
        return COMM_ERR_PARSE;
    }

    //��ȡKEY,ָ������
    cmd_rx->cmd_desc = (uint8_t *)cJSON_GetObjectItem(root,"cmd")->valuestring;  
    if(cmd_rx->cmd_desc == NULL)
    {
        printf("2.Error before: [%s]\n", cJSON_GetErrorPtr());
        return COMM_ERR_GETITEM;
    }
     printf("cmd_desc = %s\r\n",cmd_rx->cmd_desc);

    //��ȡָ����
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

    //Ŀǰָ��ֻ��1byte ����ֱ�Ӹ�ֵ
    cmd_rx->cmd = bcd_cmd[0];
    printf("cmd_rx.cmd =  %02x\r\n",cmd_rx->cmd);

    memset(tmpdat,0x00,sizeof(tmpdat));
    tmpdat = (uint8_t *)cJSON_GetObjectItem(root,"data")->valuestring;  
    dbh("tmpdat", (char *)tmpdat, 16);



    asc_len = strlen((const char*)tmpdat);

    //���������ݣ���ת��;�������򲻴���
    if(asc_len > 0)
    {
        if(asc_len % 2 != 0)
        {
            asc_len += 1;
        }
        asc2bcd(bcd_dat,tmpdat,asc_len,0);
        memcpy(cmd_rx->cmd_data,bcd_dat,asc_len/2);

        DBG("cmd_rx->cmd_data = %s\r\n",cmd_rx->cmd_data);
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
    cJSON *root; // cJSONָ��
    uint8_t len = 0;//����json�ĳ���

    uint8_t tmp_code = cmd_tx->code;
    uint8_t tmp_data[64] = {0}; 
    
    memset(tmp_data,0x00,sizeof(tmp_data));
    root=cJSON_CreateObject(); // ����root���󣬷���ֵΪcJSONָ��

    if (root == NULL)                 // ���ת�������򱨴��˳�
    {
        printf("4.Error before: [%s]\n", cJSON_GetErrorPtr());
        return 0;
    }

    //�������ǲ���ת��JSON��������ݻ�䣬ԭ��δ֪  2019.07.12 surge
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

    DBG("cmd_rx.cmd = %02x\r\n",cmd_rx->cmd);
    
    memset(&cmd_tx,0x00,sizeof(cmd_tx));
    
    switch (cmd_rx->cmd)
    {
        case GETSENSOR://��ȡ����״̬��Ŀǰֻ���������
            i = 3;
            TxdBuf[0] = STX;            
            cmd_tx.cmd = GETSENSOR;
            cmd_tx.code = 0x00;
            bsp_GetSensorStatus(cmd_tx.data);
            DBG("2cmd_tx.data = %s\r\n",cmd_tx.data);         
            i += packetJSON(&cmd_tx,tmpBuf);            
            memcpy(TxdBuf+3,tmpBuf,i-3); 
            TxdBuf[i++] = ETX;  
            
            TxdBuf[1] = i>>8; //high
            TxdBuf[2] = i&0xFF; //low
            
            iCRC = CRC16_Modbus(TxdBuf, i);  
            
            TxdBuf[i++] = iCRC >> 8;
            TxdBuf[i++] = iCRC & 0xff;            
            break;
        case SETLED: //����LED��

            break;                        
        case GETDEVICESAT://��ȡ�豸״̬

            break; 
        case GETVER:
            i = 3;
            TxdBuf[0] = STX;
            cmd_tx.cmd = GETVER;
            cmd_tx.code = 0x00;
            DBG("cmd_rx->cmd_data = %s,len = %d\r\n",cmd_rx->cmd_data,strlen((const char*)cmd_rx->cmd_data));
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
             //�������Ϳ���ָ��
            DBG("<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>\r\n");
            dbh("CONTROL MOTOR", (char *)cmd_rx->cmd_data, 8);
//            bsp_Usart4_SendData(cmd_rx->cmd_data,8);//��������
            comSendBuf(COM1,cmd_rx->cmd_data,8);
            return;
            
        case DOOR_B:
            dbh("CONTROL MOTOR door B", (char *)cmd_rx->cmd_data, 8);
//            bsp_RS485_Send_Data(cmd_rx->cmd_data,8);//����B��
            return;//���ﲻ��Ҫ����λ�����ͣ�������һ�������в�����

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



