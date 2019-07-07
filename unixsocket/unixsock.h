#ifndef UNIXSOCK_H
#define UNIXSOCK_H

#ifdef __cplusplus
extern "C"{
#endif

/*����������*/
typedef enum _NET_STREAM_TYPE_E
{
	SOCK_STREAM_E=0,//TCP
	SOCK_DGRAM_E,   //UDP
}NET_STREAM_TYPE;


/*
*�������ܣ�����unix socket������������
*���룺path-unix socket�ļ���ַ
*���룺streamType-socket����  SOCK_STREAM_E-tcp SOCK_DGRAM_E-udp
*/
int create_unix_socket(char* path,int streamType);

#ifdef __cplusplus
}
#endif

#endif