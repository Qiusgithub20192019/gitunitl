#ifndef SERVER_H
#define SERVER_H

#ifdef __cplusplus
extern "C"{
#endif

#include "pthread.h"

typedef struct SERVER_CONTEXT_S
{
	int bStart;
	int fdsock_udp;
	int fdsock_tcp;
	int enableFlag;//使能标志
	pthread_t commThd;
	pthread_mutex_t mutexlock;//线程锁
	char reserved[20];
}SERVER_CONTEXT_S;



#ifdef __cplusplus
}
#endif

#endif
