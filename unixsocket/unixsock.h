#ifndef UNIXSOCK_H
#define UNIXSOCK_H

#ifdef __cplusplus
extern "C"{
#endif

/*网络流类型*/
typedef enum _NET_STREAM_TYPE_E
{
	SOCK_STREAM_E=0,//TCP
	SOCK_DGRAM_E,   //UDP
}NET_STREAM_TYPE;


/*
*函数功能：创建unix socket，返回描述符
*输入：path-unix socket文件地址
*输入：streamType-socket类型  SOCK_STREAM_E-tcp SOCK_DGRAM_E-udp
*/
int create_unix_socket(char* path,int streamType);

#ifdef __cplusplus
}
#endif

#endif