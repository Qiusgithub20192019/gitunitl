#include "stdio.h"
#include "net/if.h"
#include "sys/un.h"
#include "sys/socket.h"
#include "errno.h"

#include "debug.h"
#include "unixsock.h"

#undef   PRT_TITLE
#define  PRT_TITLE  "unixsock"

/*
*函数功能：创建unix socket，返回描述符
*输入：path-unix socket文件地址
*输入：streamType-socket类型  SOCK_STREAM_E-tcp SOCK_DGRAM_E-udp
*/
int create_unix_socket(char* path,int streamType)
{
	struct sockaddr_un unsockaddr;
	int fdsock;
	
	if(path)
	{
		fdsock = socket(AF_UNIX,streamType,0);//SOCK_STREAM-tcp SOCK_DGRAM-udp
		if(fdsock<0)
		{
				PRT(LEVEL_ERROR,"socket create error!\n");
				return -2;
		}
			
		unsockaddr.sun_family = AF_UNIX;
		strcpy(unsockaddr.sun_path,path);
		
		unlink(path);
		
		if(bind(fdsock,(struct sockaddr*)&unsockaddr,sizeof(struct sockaddr))<0)//必须写成struct sockaddr形式
		{
				//PRT(LEVEL_ERROR,"unix socket bind failed with %s\n",perror("bind"));//这样写编译会报错
				PRT(LEVEL_ERROR,"unix socket bind failed with\n");//这样写编译会报错
				perror("bind:");
				close(fdsock);//bind failed，close socket
				return -3;
		}
		else 
		{
				PRT(LEVEL_INFO,"create unix socket success!\n");
				return fdsock;	
		}
				
	}
	else 
	{
		PRT(LEVEL_ERROR,"input unix socket path is NULL!\n");
		return -1;
	}
}
