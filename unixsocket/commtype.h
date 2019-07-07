#ifndef COMMTYPE_H
#define COMMTYPE_H

#define MAX_BUFFER_SIZE   2048

/*ͨ��Э��ͷ�ṹ��*/
typedef struct _COMM_HEAD_S
{
	int sourceID;
	int destID;
	int cmd;
	int sessionID;
	int datalen;
	char reserved[4];
}COMM_HEAD_S;

/*ͨ��Э�����ṹ��*/
typedef struct _COMM_S
{
	COMM_HEAD_S head;
	unsigned char data[MAX_BUFFER_SIZE];	
}COMM_S;

/*�����ֶ���*/
typedef enum _CMD_CODE_E
{
	CMD_STOP=0,
	CMD_START,
	CMD_DATA,
}CMD_CODE_E;

/*ʹ��״̬����*/
typedef enum _STATUS_E
{
	DISABLE = 0,
	ENABLE,
}STATUS_E;

#define CLIENT_UNIXSOCK_ADDR_UDP  "/tmp/client_unsock_udp"
#define CLIENT_UNIXSOCK_ADDR_TCP  "/tmp/client_unsock_tcp"
#define SERVER_UNIXSOCK_ADDR_UDP  "/tmp/server_unsock_udp"
#define SERVER_UNIXSOCK_ADDR_TCP  "/tmp/server_unsock_tcp"

#endif