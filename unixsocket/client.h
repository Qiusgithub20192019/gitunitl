//test
#ifndef CLINET_H
#define CLINET_H

#include "pthread.h"

typedef struct _CLIENT_CONTEXT_S
{
	int bStart;
	int fdsock_udp;
	int fdsock_tcp;
	int enableFlag;//使能标志
	pthread_t commThd;
	pthread_mutex_t mutexlock;//线程锁
	char reserved[20];
}CLIENT_CONTEXT_S;



#endif
