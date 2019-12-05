#include "lwip/netdb.h"

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "string.h"




#define LOG_TAG    "TRANSPORT"
#include "elog.h"
#include "transport.h"


static int mysock;

/************************************************************************
** 函数名称: transport_sendPacketBuffer									
** 函数功能: 以TCP方式发送数据
** 入口参数: unsigned char* buf：数据缓冲区
**           int buflen：数据长度
** 出口参数: <0发送数据失败							
************************************************************************/
int32_t transport_sendPacketBuffer( uint8_t* buf, int32_t buflen)
{
	int32_t rc;
	rc = write(mysock, buf, buflen);
	return rc;
}

/************************************************************************
** 函数名称: transport_getdata									
** 函数功能: 以阻塞的方式接收TCP数据
** 入口参数: unsigned char* buf：数据缓冲区
**           int count：数据长度
** 出口参数: <=0接收数据失败									
************************************************************************/
int32_t transport_getdata(uint8_t* buf, int32_t count)
{
	int32_t rc;
	//这个函数在这里不阻塞
  rc = recv(mysock, buf, count, 0);
	return rc;
}


//ip = -570995080,port = 23303
/************************************************************************
** 函数名称: transport_open									
** 函数功能: 打开一个接口，并且和服务器 建立连接
** 入口参数: char* servip:服务器域名
**           int   port:端口号
** 出口参数: <0打开连接失败										
************************************************************************/
int32_t transport_open(int8_t* servip, int32_t port)
{        
    int32_t *sock = &mysock;

    struct sockaddr_in client_addr;        
    ip_addr_t ipaddr;


    IP4_ADDR(&ipaddr,120,78,247,221);


    *sock= socket(AF_INET, SOCK_STREAM, 0);
    if (*sock < 0)
    {
        printf("Socket error\n");
        vTaskDelay(10);   
    } 

    client_addr.sin_family = AF_INET;      
    client_addr.sin_port = htons(1883);   
    client_addr.sin_addr.s_addr = ipaddr.addr;
    memset(&(client_addr.sin_zero), 0, sizeof(client_addr.sin_zero));    
    printf("ip = %d,port = %d\r\n",client_addr.sin_addr.s_addr,client_addr.sin_port);
    if (connect(*sock, 
             (struct sockaddr *)&client_addr, 
              sizeof(struct sockaddr)) == -1) 
    {
      printf("Connect failed!\n");
      closesocket(*sock);
      vTaskDelay(10);
      return -1;
    }                                           

    printf("Connect to server successful!\n");

    //返回套接字
	return *sock;
}


/************************************************************************
** 函数名称: transport_close									
** 函数功能: 关闭套接字
** 入口参数: unsigned char* buf：数据缓冲区
**           int buflen：数据长度
** 出口参数: <0发送数据失败							
************************************************************************/
int transport_close(void)
{
  
	int rc;
//	rc = close(mysock);
  rc = shutdown(mysock, SHUT_WR);
	rc = recv(mysock, NULL, (size_t)0, 0);
	rc = close(mysock);
	return rc;
}


//int transport_open(char* addr, int port)
//{
//	int* sock = &mysock;
//	struct hostent *server;
//	struct sockaddr_in serv_addr;
//	static struct  timeval tv;
//	int timeout = 1000;
//	fd_set readset;
//	fd_set writeset;
//	*sock = socket(AF_INET, SOCK_STREAM, 0);
//	if(*sock < 0)
//		log_d("Create socket failed\n");
//    
//	log_d("Create socket success\n");
//	server = gethostbyname(addr);
//	if(server == NULL)
//		log_d("Get host ip failed\n");

//    log_d("Get host ip success\n");
//    
//	memset(&serv_addr,0,sizeof(serv_addr));
//	serv_addr.sin_family = AF_INET;
//	serv_addr.sin_port = htons(port);
//     log_d("sin_port = %d\r\n",serv_addr.sin_port);
//     
//	memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

//     log_d("sin_addr = %d\r\n",serv_addr.sin_addr.s_addr);
//     
//	if(connect(*sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
//	{
//		log_d("connect failed\n");
//        return -1;
//	}
//    log_d("connect success\n");
//    
//	tv.tv_sec = 10;  /* 1 second Timeout */
//	tv.tv_usec = 0; 
//	setsockopt(mysock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout,sizeof(timeout));

//    return mysock;

//}



