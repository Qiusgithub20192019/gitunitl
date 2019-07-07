#include "stdio.h"
#include "net/if.h"
#include "sys/socket.h"
#include "sys/un.h"
#include "sys/time.h"
#include "fcntl.h"
#include "pthread.h"

#include "server.h"
#include "debug.h"
#include "commtype.h"

#undef  PRT_TITLE
#define PRT_TITLE "server"

SERVER_CONTEXT_S g_serverContext;

/*
*函数功能：客户端程序初始化
*输入：客户端程序主数据结构体
*返回:0-成功 <0-失败
*/
int ClintInit(SERVER_CONTEXT_S*pCliContext)
{
	if(pCliContext)
	{
			pCliContext->fdsock_udp = create_unix_socket(SERVER_UNIXSOCK_ADDR_UDP,SOCK_DGRAM);
			if(pCliContext->fdsock_udp<0)
			{
				pCliContext->fdsock_udp = 0;//必须清楚
				PRT(LEVEL_ERROR,"server Initial udp socket create failed!\n");
				return -2;				
			}
			else PRT(LEVEL_INFO,"server Initial udp socket create success,fd is %d\n",pCliContext->fdsock_udp);
				
			/*pCliContext->fdsock_tcp= create_unix_socket(SERVER_UNIXSOCK_ADDR_TCP,SOCK_STREAM);
			if(pCliContext->fdsock_tcp<0)
			{
				PRT(LEVEL_ERROR,"server Initial tcp socket create failed!\n");
				return -3;				
			}		*/	
			pthread_mutex_init(&pCliContext->mutexlock,NULL);//初始化线程锁
			PRT(LEVEL_INFO,"server unix Initial!\n");
			return 0;
	}
	else 
	{
		PRT(LEVEL_ERROR,"server Initial input parameter is NULL!\n");
		return -1;
	}
}

/*
*函数功能：客户端程序去 初始化
*输入：客户端程序主数据结构体
*返回:0-成功 <0-失败
*/
int ClintExit(SERVER_CONTEXT_S*pCliContext)
{
	if(pCliContext)
		{
			if(pCliContext->fdsock_udp)
				close(pCliContext->fdsock_udp);
			if(pCliContext->fdsock_tcp)
				close(pCliContext->fdsock_tcp);
				
			pthread_mutex_destroy(&pCliContext->mutexlock);//销毁线程锁
			PRT(LEVEL_INFO,"server unix Exit!\n");
			
		}
	return 0;
}
/*
*函数功能：解析并处理通信协议
*输入：pCliContext-进程全局数据结构体指针
*输入：data-通信接收过来的数据 datalen-接收数据长度
*返回：0-处理成功 <0-处理失败
*日期：2018-07-28
*作者：qiu
*/
int deal_commData(SERVER_CONTEXT_S*pCliContext,unsigned char*data,int datalen)
{
	COMM_S *pComm = (COMM_S*)data;
	if((data)&&(datalen>0))
	{
			PRT(LEVEL_INFO,"receive [%#.2x] command\n",pComm->head.cmd);
			switch (pComm->head.cmd)
			{
				case CMD_START:
					 PRT(LEVEL_INFO,"open status\n");
					 pthread_mutex_lock(&pCliContext->mutexlock);
					 pCliContext->enableFlag = ENABLE;
					 pthread_mutex_unlock(&pCliContext->mutexlock);
				break;
				case CMD_STOP:				
					 PRT(LEVEL_INFO,"close status\n");
					 pthread_mutex_lock(&pCliContext->mutexlock);
					 pCliContext->enableFlag = DISABLE;
					 pthread_mutex_unlock(&pCliContext->mutexlock);
				break;
				case CMD_DATA:
					PRT(LEVEL_INFO,"receive data......,data length is %d bytes,the session id is %d\n",pComm->head.datalen,pComm->head.sessionID);
				break;
				
				default:PRT(LEVEL_WARN,"unknow command[%#.2x]\n",pComm->head.cmd);
					
			}
	}
	else 
	{
		
	}
}

/*
*函数功能：接收UDP unix socket 通信数据
*输入：线程参数
*/
void*CommThreadFunc(void*arg)
{
	SERVER_CONTEXT_S *pCliContext = (SERVER_CONTEXT_S*)arg;
	int ret;
	struct timeval to;
	fd_set readset;
	int fd_max;
	unsigned char buffer[MAX_BUFFER_SIZE]={0};
	int recvlen,datalen;
	COMM_HEAD_S *pCommHead;
	
	to.tv_sec = 1;
	to.tv_usec = 0;
	
	fd_max = pCliContext->fdsock_udp+1;
	
	while(pCliContext->bStart)
	{
		FD_ZERO(&readset);
		FD_SET(pCliContext->fdsock_udp,&readset);
		
		ret = select(fd_max,&readset,NULL,NULL,&to);
		if(ret<0)
		{
				PRT(LEVEL_ERROR,"select occur error!\n");
				break;
		}
		else if(ret==0)
		{
				//sleep(1);
				//PRT(LEVEL_ERROR,"select timeout!\n");
				usleep(100000);
				continue;
		}
		else 
		{
			if(FD_ISSET(pCliContext->fdsock_udp,&readset))
			{
				recvlen = sizeof(COMM_HEAD_S);
				ret = recv(pCliContext->fdsock_udp,buffer,recvlen,0);
				if(ret == recvlen)
				{
						pCommHead = (COMM_HEAD_S*)buffer;
						datalen = pCommHead->datalen;
						if(datalen>0)
						{
								ret = recv(pCliContext->fdsock_udp,buffer+recvlen,datalen,0);
								//if(ret == datalen)
								{
									PRT(LEVEL_INFO,"receive %d bytes data ok!\n",ret);									
								}
								
						}
						deal_commData(pCliContext,buffer,recvlen+ret);///待实现
				}
				else 
				{
					PRT(LEVEL_ERROR,"receive length error!\n");
					continue;
				}
			}
		
		}
	}
}

/*
*函数功能：创建通信线程
*输入：pCliContext-进程全局数据结构体
*返回：0-成功 <0-失败
*/
int CreateCommThread(SERVER_CONTEXT_S*pCliContext)
{
	if(pCliContext)
	{
		pthread_create(&pCliContext->commThd,NULL,CommThreadFunc,pCliContext);
		PRT(LEVEL_INFO,"create communication thread success!\n");
		return 0;
	}
	else 
	{
		PRT(LEVEL_ERROR,"create communication thread failed! input parameter is NULL!\n");
		return -1;
	}
}

/*
*函数功能：退出通信线程
*输入：pCliContext-进程全局数据结构体
*返回：0-成功 <0-失败
*/
int ExitCommThread(SERVER_CONTEXT_S*pCliContext)
{
	if(pCliContext)
	{
		pthread_join(pCliContext->commThd,NULL);
		PRT(LEVEL_INFO,"exit communication thread success\n");
		return 0;
	}
	else 
	{
		PRT(LEVEL_ERROR,"exit communication thread failed! input parameter is NULL!\n");
		return -1;
	}
}


int main(int argn,char**argv)
{
	int ret;
	
	ret =ClintInit(&g_serverContext);
	if(ret<0)
	{
		PRT(LEVEL_ERROR,"Server do failed!\n");
		return -1;
	}
	
	g_serverContext.bStart = 1;
	
	CreateCommThread(&g_serverContext);
	
	while (g_serverContext.bStart) 
	{
		 sleep(1);
	}
	
	ExitCommThread(&g_serverContext);
	
	return 0;
}