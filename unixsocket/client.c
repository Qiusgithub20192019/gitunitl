#include "stdio.h"
#include "net/if.h"
#include "sys/socket.h"
#include "sys/un.h"
#include "sys/time.h"
#include "fcntl.h"
#include "pthread.h"
#include "errno.h"

#include "client.h"
#include "debug.h"
#include "commtype.h"

#undef   PRT_TITLE
#define  PRT_TITLE  "client"

CLIENT_CONTEXT_S g_clientContext;

/*
*�������ܣ��ͻ��˳����ʼ��
*���룺�ͻ��˳��������ݽṹ��
*����:0-�ɹ� <0-ʧ��
*/
int ClintInit(CLIENT_CONTEXT_S*pCliContext)
{
	if(pCliContext)
	{
			pCliContext->fdsock_udp = create_unix_socket(CLIENT_UNIXSOCK_ADDR_UDP,SOCK_DGRAM);
			if(pCliContext->fdsock_udp<0)
			{
				pCliContext->fdsock_udp = 0;//�������
				PRT(LEVEL_ERROR,"client Initial udp socket create failed!\n");
				return -2;				
			}
			else PRT(LEVEL_INFO,"client Initial udp socket create success,fd is %d\n",pCliContext->fdsock_udp);
				
			/*pCliContext->fdsock_tcp= create_unix_socket(CLIENT_UNIXSOCK_ADDR_TCP,SOCK_STREAM);
			if(pCliContext->fdsock_tcp<0)
			{
				PRT(LEVEL_ERROR,"client Initial tcp socket create failed!\n");
				return -3;				
			}	*/		
			pthread_mutex_init(&pCliContext->mutexlock,NULL);//��ʼ���߳���
			PRT(LEVEL_INFO,"Client unix Initial!\n");
			return 0;
	}
	else 
	{
		PRT(LEVEL_ERROR,"client Initial input parameter is NULL!\n");
		return -1;
	}
}

/*
*�������ܣ��ͻ��˳���ȥ ��ʼ��
*���룺�ͻ��˳��������ݽṹ��
*����:0-�ɹ� <0-ʧ��
*/
int ClintExit(CLIENT_CONTEXT_S*pCliContext)
{
	if(pCliContext)
		{
			if(pCliContext->fdsock_udp)
				close(pCliContext->fdsock_udp);
			if(pCliContext->fdsock_tcp)
				close(pCliContext->fdsock_tcp);
				
			pthread_mutex_destroy(&pCliContext->mutexlock);//�����߳���
			PRT(LEVEL_INFO,"Client unix Exit!\n");
			
		}
	return 0;
}
/*
*�������ܣ�����������ͨ��Э��
*���룺pCliContext-����ȫ�����ݽṹ��ָ��
*���룺data-ͨ�Ž��չ��������� datalen-�������ݳ���
*���أ�0-����ɹ� <0-����ʧ��
*���ڣ�2018-07-28
*���ߣ�qiu
*/
int deal_commData(CLIENT_CONTEXT_S*pCliContext,unsigned char*data,int datalen)
{
	COMM_S *pComm = (COMM_S*)data;
	if((data)&&(datalen>0))
	{
			PRT(LEVEL_INFO,"receive [%#.2x] command\n");
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
				
				default:PRT(LEVEL_WARN,"unknow command[%#.2x]\n");
					
			}
	}
	else 
	{
		
	}
}

/*
*�������ܣ�����UDP unix socket ͨ������
*���룺�̲߳���
*/
void*CommThreadFunc(void*arg)
{
	CLIENT_CONTEXT_S *pCliContext = (CLIENT_CONTEXT_S*)arg;
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
				usleep(10000);
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
						deal_commData(pCliContext,buffer,recvlen+ret);///��ʵ��
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
*�������ܣ�����ͨ���߳�
*���룺pCliContext-����ȫ�����ݽṹ��
*���أ�0-�ɹ� <0-ʧ��
*/
int CreateCommThread(CLIENT_CONTEXT_S*pCliContext)
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
*�������ܣ��˳�ͨ���߳�
*���룺pCliContext-����ȫ�����ݽṹ��
*���أ�0-�ɹ� <0-ʧ��
*/
int ExitCommThread(CLIENT_CONTEXT_S*pCliContext)
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

/*
*�������ܣ����������������
*���룺pCliContext-�ͻ���ȫ�ֱ����ṹ��
*���룺pdata-��������ָ��
*���룺len-���ݳ���
*���أ�0-�ɹ� <0-ʧ��
*
*/
int SendDataToServer(CLIENT_CONTEXT_S*pCliContext,unsigned char*pdata,int len)
{
	struct sockaddr_un seraddr;
	COMM_S commData;
	int ret;
	static count=0;
	
	if(pCliContext && pdata)
	{
			if(pCliContext->fdsock_udp)
			{
				//׼����������ַ
				seraddr.sun_family = AF_UNIX;
				strcpy(seraddr.sun_path,SERVER_UNIXSOCK_ADDR_UDP);
				
				//׼������
				memset(&commData.head,0,sizeof(commData.head));
				commData.head.cmd = CMD_DATA;
				commData.head.sessionID = ++count;
				commData.head.datalen = len>MAX_BUFFER_SIZE?MAX_BUFFER_SIZE:len;
				
				memcpy(commData.data,pdata,commData.head.datalen);
				
				//����
				ret = sendto(pCliContext->fdsock_udp,&commData,sizeof(commData.head)+commData.head.datalen,0,(struct sockaddr*)&seraddr,sizeof(struct sockaddr));
				if(ret ==sizeof(commData.head)+commData.head.datalen)
				{
					PRT(LEVEL_INFO,"sessionID %d send ok!\n",commData.head.sessionID);
				}
				else 
				{
					PRT(LEVEL_ERROR,"sessionID %d send failed with ret=%d\n",commData.head.sessionID,ret);
					perror("sendto");
					return -3;
				}
					
			}
			else 
			{
				PRT(LEVEL_ERROR,"It is not build socket!\n");
				return -2;
			}
	}
	else 
	{
		PRT(LEVEL_ERROR,"input parameter is NULL!\n");
		return -1;
	}
	
	return 0;
}

int main(int argn,char**argv)
{
	int ret;
	char*statement="this is client,hello! server\n";
	
	ret =ClintInit(&g_clientContext);
	if(ret<0)
	{
		
	}
	
	g_clientContext.bStart = 1;
	
	CreateCommThread(&g_clientContext);
	
	while (g_clientContext.bStart) 
	{
		 sleep(1);
		 SendDataToServer(&g_clientContext,statement,strlen(statement));
	}
	
	ExitCommThread(&g_clientContext);
	
	return 0;
}