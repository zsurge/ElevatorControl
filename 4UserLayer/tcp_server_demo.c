#include "tcp_server_demo.h"
#include "lwip/opt.h"
#include "lwip_comm.h"
#include "lwip/lwip_sys.h"
#include "lwip/api.h" 
 

u8 tcp_server_recvbuf[TCP_SERVER_RX_BUFSIZE];	//TCP客户端接收数据缓冲区
u8 *tcp_server_sendbuf="Explorer STM32F407 NETCONN TCP Server send data\r\n";	
u8 tcp_server_flag;								//TCP服务器数据发送标志位

//设置任务优先级
#define TCPSERVER_PRIO		7
//任务堆栈大小	
#define TCPSERVER_STK_SIZE 		512  
TaskHandle_t  TCPSERVER_TASK_Handler;
//任务函数
static void Tcp_Server_task(void *pvParameters);


//tcp服务器任务
static void Tcp_Server_task(void *arg)
{
	u32 data_len = 0;
	struct pbuf *q;
	err_t err,recv_err;
	u8 remot_addr[4];
	struct netconn *conn, *newconn;
	static ip_addr_t ipaddr;
	static u16_t 			port;
	
	LWIP_UNUSED_ARG(arg);

	conn = netconn_new(NETCONN_TCP);  //创建一个TCP链接
	netconn_bind(conn,IP_ADDR_ANY,TCP_SERVER_PORT);  //绑定端口
	netconn_listen(conn);  		//进入监听模式
	conn->recv_timeout = 10;  	//禁止阻塞线程 等待10ms
	while (1) 
	{
		err = netconn_accept(conn,&newconn);  //接收连接请求
		if(err==ERR_OK)newconn->recv_timeout = 10;

		if (err == ERR_OK)    //处理新连接的数据
		{ 
			struct netbuf *recvbuf;

			netconn_getaddr(newconn,&ipaddr,&port,0); //获取远端IP地址和端口号
			
			remot_addr[3] = (uint8_t)(ipaddr.addr >> 24); 
			remot_addr[2] = (uint8_t)(ipaddr.addr>> 16);
			remot_addr[1] = (uint8_t)(ipaddr.addr >> 8);
			remot_addr[0] = (uint8_t)(ipaddr.addr);
			printf("主机%d.%d.%d.%d连接上服务器,主机端口号为:%d\r\n",remot_addr[0], remot_addr[1],remot_addr[2],remot_addr[3],port);
			
			while(1)
			{
				if((tcp_server_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA) //有数据要发送
				{
					err = netconn_write(newconn ,tcp_server_sendbuf,strlen((char*)tcp_server_sendbuf),NETCONN_COPY); //发送tcp_server_sendbuf中的数据
					if(err != ERR_OK)
					{
						printf("发送失败\r\n");
					}
					tcp_server_flag &= ~LWIP_SEND_DATA;
				}
				
				if((recv_err = netconn_recv(newconn,&recvbuf)) == ERR_OK)  	//接收到数据
				{		
					taskENTER_CRITICAL(); //关中断
					memset(tcp_server_recvbuf,0,TCP_SERVER_RX_BUFSIZE);  //数据接收缓冲区清零
					for(q=recvbuf->p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
					{
						//判断要拷贝到TCP_SERVER_RX_BUFSIZE中的数据是否大于TCP_SERVER_RX_BUFSIZE的剩余空间，如果大于
						//的话就只拷贝TCP_SERVER_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
						if(q->len > (TCP_SERVER_RX_BUFSIZE-data_len)) memcpy(tcp_server_recvbuf+data_len,q->payload,(TCP_SERVER_RX_BUFSIZE-data_len));//拷贝数据
						else memcpy(tcp_server_recvbuf+data_len,q->payload,q->len);
						data_len += q->len;  	
						if(data_len > TCP_SERVER_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出	
					}
					taskEXIT_CRITICAL();  //开中断
					data_len=0;  //复制完成后data_len要清零。	
					printf("%s\r\n",tcp_server_recvbuf);  //通过串口发送接收到的数据
					netbuf_delete(recvbuf);
				}else if(recv_err == ERR_CLSD)  //关闭连接
				{
					netconn_close(newconn);
					netconn_delete(newconn);
					printf("主机:%d.%d.%d.%d断开与服务器的连接\r\n",remot_addr[0], remot_addr[1],remot_addr[2],remot_addr[3]);
					break;
				}
			}
		}
	}
}


//创建TCP服务器线程
//返回值:0 TCP服务器创建成功
//		其他 TCP服务器创建失败
void tcp_server_init(void)
{
	taskENTER_CRITICAL();      			//进入临界区
	//创建DHCP任务 
	xTaskCreate((TaskFunction_t )Tcp_Server_task,     	
              (const char*    )"lwip_tcp_task",   	
              (uint16_t       )TCPSERVER_STK_SIZE, 
              (void*          )NULL,				
              (UBaseType_t    )TCPSERVER_PRIO,	
              (TaskHandle_t*  )&TCPSERVER_TASK_Handler);  
	taskEXIT_CRITICAL();            //退出临界区

}


