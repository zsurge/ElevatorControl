/**********************************************************************************************************
** �ļ���		:mqtt_app.c
** ����			:maxlicheng<licheng.chn@outlook.com>
** ����github	:https://github.com/maxlicheng
** ���߲���		:https://www.maxlicheng.com/	
** ��������		:2018-08-08
** ����			:mqtt�������
************************************************************************************************************/
#include "mqtt_app.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "FreeRTOS.h"
#include "task.h"
#include "malloc.h"
#include <string.h>
#include <stdio.h>
#include "cJSON.h"

#define LOG_TAG    "MQTTAPP"
#include "elog.h"


#define send_duration	20	//�¶ȷ������ڣ�ms��

float temp = 0;
float humid = 0;

static uint32_t send_cnt = 0;
static uint32_t recv_cnt = 0;


void mqtt_thread(void)
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	MQTTString receivedTopic;
	MQTTString topicString = MQTTString_initializer;
	
	int32_t rc = 0;
	unsigned char buf[1024];
	int buflen = sizeof(buf);
	int	mysock = 0;
	
	int payloadlen_in;
	unsigned char* payload_in;
	unsigned short msgid = 1;
	int subcount;
	int granted_qos =0;
	unsigned char sessionPresent, connack_rc;
	unsigned short submsgid;
	uint32_t len = 0;
	int req_qos = 0;
	unsigned char dup;
	int qos;
	unsigned char retained = 0;

	uint8_t connect_flag = 0;		//���ӱ�־
	uint8_t msgtypes = CONNECT;		//��Ϣ״̬��ʼ��
	uint8_t t=0;
	
	log_d("socket connect to server\r\n");
	mysock = transport_open((char *)HOST_NAME,HOST_PORT);
	log_d("Sending to hostname %s port %d\r\n", HOST_NAME, HOST_PORT);
	
	len = MQTTSerialize_disconnect((unsigned char*)buf,buflen);
	rc = transport_sendPacketBuffer(mysock, (uint8_t *)buf, len);
	if(rc == len)															//
		log_d("send DISCONNECT Successfully\r\n");
	else
		log_d("send DISCONNECT failed\r\n"); 
    
	vTaskDelay(2500);
	
	log_d("socket connect to server\r\n");
	mysock = transport_open((char *)HOST_NAME,HOST_PORT);
	log_d("Sending to hostname %s port %d\r\n", HOST_NAME, HOST_PORT);


	data.clientID.cstring = CLIENT_ID;                   //���
	data.keepAliveInterval = KEEPLIVE_TIME;         //���ֻ�Ծ
	data.username.cstring = USER_NAME;              //�û���
	data.password.cstring = PASSWORD;               //��Կ
	data.MQTTVersion = MQTT_VERSION;                //3��ʾ3.1�汾��4��ʾ3.11�汾
	data.cleansession = 1;
    
	unsigned char payload_out[200];
	int payload_out_len = 0;

    //��ȡ��ǰ�δ���Ϊ��������ʼʱ��
	uint32_t curtick  =	 xTaskGetTickCount();
	uint32_t sendtick =  xTaskGetTickCount();
    
	while(1)
	{
		if(( xTaskGetTickCount() - curtick) >(data.keepAliveInterval*200))		//ÿ��200��tick
		{
			if(msgtypes == 0)
			{                
				curtick =  xTaskGetTickCount();
				msgtypes = PINGREQ;
                log_d("send heartbeat!!  set msgtypes = %d \r\n",msgtypes);
			}

		}
		if(connect_flag == 1)
		{
			if((xTaskGetTickCount() - sendtick) >= (send_duration*200))
			{
                log_d("send PUBLISH!!  get msgtypes = %d \r\n",msgtypes);
                
				sendtick = xTaskGetTickCount();
                
				taskENTER_CRITICAL();	//�����ٽ���(�޷����жϴ��)
				temp =10.0;
				taskEXIT_CRITICAL();		//�˳��ٽ���(���Ա��жϴ��)
				
				humid = 54.8+rand()%10+1;
//				sprintf((char*)payload_out,"{\"params\":{\"CurrentTemperature\":+%0.1f,\"RelativeHumidity\":%0.1f},\"method\":\"thing.event.property.post\"}",temp, humid);
                sprintf((char*)payload_out,"{\"commandCode\":\"1000\",\"data\":{\"CurrentTemperature\":\"%0.1f\",\"RelativeHumidity\":\"%0.1f\"},\"dev\":\"arm test\"}",temp, humid);

				payload_out_len = strlen((char*)payload_out);
				topicString.cstring = DEVICE_PUBLISH;		//�����ϱ� ����
				log_d("send PUBLISH buff = %s\r\n",payload_out);
				len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, req_qos, retained, msgid, topicString, payload_out, payload_out_len);//������Ϣ
				rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
				if(rc == len)															//
					log_d("the %dth send PUBLISH Successfully\r\n",send_cnt++);
				else
					log_d("send PUBLISH failed\r\n");  
				log_d("send temp(%0.1f)&humid(%0.1f) !\r\n",temp, humid);
			}
		}
		switch(msgtypes)
		{

            //���ӷ���� �ͻ����������ӷ����
            case CONNECT:	len = MQTTSerialize_connect((unsigned char*)buf, buflen, &data); 						//��ȡ�����鳤��		����������Ϣ     
							rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);		//���� ���ط������鳤��
							if(rc == len)															//
								log_d("send CONNECT Successfully\r\n");
							else
								log_d("send CONNECT failed\r\n");               
							log_d("step = %d,MQTT concet to server!\r\n",CONNECT);
							msgtypes = 0;
							break;
           //ȷ����������
			case CONNACK:   if(MQTTDeserialize_connack(&sessionPresent, &connack_rc, (unsigned char*)buf, buflen) != 1 || connack_rc != 0)	//�յ���ִ
							{
								log_d("Unable to connect, return code %d\r\n", connack_rc);		//��ִ��һ�£�����ʧ��
							}
							else
							{
								log_d("step = %d,MQTT is concet OK!\r\n",CONNACK);									//���ӳɹ�
								connect_flag = 1;
							}
							msgtypes = SUBSCRIBE;													//���ӳɹ� ִ�� ���� ����
							break;
            //�������� �ͻ��˶�������
			case SUBSCRIBE: topicString.cstring = DEVICE_SUBSCRIBE;
							len = MQTTSerialize_subscribe((unsigned char*)buf, buflen, 0, msgid, 1, &topicString, &req_qos);
							rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
							if(rc == len)
								log_d("send SUBSCRIBE Successfully\r\n");
							else
							{
								log_d("send SUBSCRIBE failed\r\n"); 
								t++;
								if(t >= 10)
								{
									t = 0;
									msgtypes = CONNECT;
								}
								else
									msgtypes = SUBSCRIBE;
								break;
							}
							log_d("step = %d,client subscribe:[%s]\r\n",SUBSCRIBE,topicString.cstring);
							msgtypes = 0;
							break;
            //����ȷ�� ����������ȷ��
			case SUBACK: 	rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, (unsigned char*)buf, buflen);	//�л�ִ  QoS                                                     
							log_d("step = %d,granted qos is %d\r\n",SUBACK, granted_qos);         								//��ӡ Qos                                                       
							msgtypes = 0;
							break;
            //������Ϣ
			case PUBLISH:	rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,&payload_in, &payloadlen_in, (unsigned char*)buf, buflen);	//��ȡ������������Ϣ
							log_d("step = %d,message arrived : %s\r\n",PUBLISH,payload_in);
							cJSON *json , *json_params, *json_id, *json_led, *json_display;
							json = cJSON_Parse((char *)payload_in);			//�������ݰ�
							if (!json)  
							{  
								log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
							} 
							else
							{
								json_id = cJSON_GetObjectItem(json , "commandCode"); 
								if(json_id->type == cJSON_String)
								{
									log_d("commandCode:%s\r\n", json_id->valuestring);  
								}
								json_params = cJSON_GetObjectItem( json , "data" );

                                json_led = cJSON_GetObjectItem( json_params , "userId" );

                                json_display = cJSON_GetObjectItem( json_params , "startTime" );
  
                			  log_d("code:%s ;   userId:%d ;   startTime:%s ,qos = %d\r\n",
                              json_id->valuestring,
                              json_led->valueint,
                              json_display->valuestring,
                              qos);
                              
                								 
							}
							cJSON_Delete(json);
                            cJSON_Delete(json_params);
                            cJSON_Delete(json_id);
                            cJSON_Delete(json_display);
                            log_d("the %dth recv \r\n",recv_cnt++);
							
							if(qos == 1)
							{
								log_d("publish qos is 1,send publish ack.\r\n");							//QosΪ1�����л�ִ ��Ӧ
								memset(buf,0,buflen);
								len = MQTTSerialize_ack((unsigned char*)buf,buflen,PUBACK,dup,msgid);   					//publish ack                       
								rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);			//
								if(rc == len)
									log_d("send PUBACK Successfully\r\n");
								else
									log_d("send PUBACK failed\r\n");                                       
							}
							msgtypes = 0;
							break;
            //����ȷ�� QoS	1��Ϣ�����յ�ȷ��
			case PUBACK:    log_d("step = %d,PUBACK!\r\n",PUBACK);					//�����ɹ�
							msgtypes = 0;
							break;
            //�����յ� QoS2 ��һ��
			case PUBREC:    log_d("step = %d,PUBREC!\r\n",PUBREC);     				//just for qos2
							break;
            //�����ͷ� QoS2 �ڶ���
			case PUBREL:    log_d("step = %d,PUBREL!\r\n",PUBREL);        			//just for qos2
							break;
            //������� QoS2 ������                            
			case PUBCOMP:   log_d("step = %d,PUBCOMP!\r\n",PUBCOMP);        			//just for qos2
							break;
            //��������
			case PINGREQ:   len = MQTTSerialize_pingreq((unsigned char*)buf, buflen);							//����
							rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
							if(rc == len)
								log_d("send PINGREQ Successfully\r\n");
							else
								log_d("send PINGREQ failed\r\n");       
								log_d("step = %d,time to ping mqtt server to take alive!\r\n",PINGREQ);
								msgtypes = 0;
							break;
            //������Ӧ
			case PINGRESP:	log_d("step = %d,mqtt server Pong\r\n",PINGRESP);  			//������ִ����������Ӧ                                                     
							msgtypes = 0;
							break;
			default:
							break;

		}
		memset(buf,0,buflen);
		rc=MQTTPacket_read((unsigned char*)buf, buflen, transport_getdata);//��ѯ����MQTT�������ݣ�
		log_d("MQTTPacket_read = %d\r\n",rc);		
		if(rc >0)//��������ݣ�������Ӧ״̬��
		{
			msgtypes = rc;
			log_d("MQTT is get recv: msgtypes = %d\r\n",msgtypes);
		}
	}
	transport_close(mysock);
    log_d("mqtt thread exit.\r\n");
}



/*
// C prototype : void HexToStr(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - ���Ŀ���ַ���
// [IN] pbSrc - ����16����������ʼ��ַ
// [IN] nLen - 16���������ֽ���
// return value: 
// remarks : ��16������ת��Ϊ�ַ���
*/
void HexToStr(uint8_t *pbDest, uint8_t *pbSrc, int nLen)
{
	char ddl,ddh;
	int i;

	for (i=0; i<nLen; i++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		pbDest[i*2] = ddh;
		pbDest[i*2+1] = ddl;
	}

	pbDest[nLen*2] = '\0';
}



