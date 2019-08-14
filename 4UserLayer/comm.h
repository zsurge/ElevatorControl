/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : comm.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��6��18��
  ����޸�   :
  ��������   : ����ͨѶЭ�����������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��6��18��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __COMM_H
#define __COMM_H

#include "tool.h"
#include "cJSON.h"
#include "bsp_infrared.h"


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
//#define STX		    					0x02	/* frame head */
#define ETX                             0x03    /* frame tail */
#define CMDSUCC	    					0x06	/* command success */
#define CMDERRO	    					0x15	/* command error */
#define UPDAT							0x13	/* Upload data */
    
    
#define SPACE		        			0x00
#define FINISH		       	 			0x55

#define GETSENSOR                       0xA1
#define SETLED                          0xA2
#define GETDEVICESAT                    0xA3
#define GETVER                          0xA4
#define UPSENSOR                        0xA5
#define HEARTBEAT                       0xA6
#define UPGRADE                         0xA7
#define CONTROLMOTOR                    0xA8

#define WGREADER                        0xA9
#define QRREADER                        0xA0

#define DOOR_B                          0xB8


#define DEV_MOTOR                       0x01
#define DEV_SENSOR                      0x02
#define DEV_HEARTBEAT                   0x03


#define JSON_PACK_MAX                   512

#define CMD_SERIAL_PORT                 0x01

typedef enum
{
    NoCMD = 0xA0,
    GetSensor = 0xA1,
    SetLed,
    GetDeviceStatus,
    GetVersion,
    UpGradeApp    
}CommandType;

typedef enum {
    COMM_SUCCESS = 0,                                       /**< success */
    COMM_ERR_PACKET = 1,                                    /**< cJSON packet error*/
    COMM_ERR_PARSE = 2,                                     /**< cJSON parse error*/
    COMM_ERR_GETITEM = 3                                    /**< get cJSON item error*/ 
}COMM_ERR;  
    

typedef struct
{
    uint8_t cmd;     //ָ����
    uint8_t *cmd_desc;//ָ������    
    uint8_t cmd_data[64];//ָ������(����)
}CMD_RX_T;

static CMD_RX_T gcmd_rx;

typedef struct
{
    uint8_t cmd;         //ָ����
    uint8_t code;        //ִ��״̬   
    uint8_t data[64];       //ָ�������(����)
}CMD_TX_T;

static CMD_TX_T gcmd_tx;


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
void deal_rx_data(void);
void deal_Serial_Parse(void);
void init_serial_boot(void);
uint8_t send_to_host(uint8_t cmd,uint8_t *buf,uint8_t len);
void send_to_device(CMD_RX_T *cmd_rx);

#endif

