#ifndef __TPOOLCORE_H__
#define __TPOOLCORE_H__

#include <pthread.h>
#include "workerlist.h"
#include "Tpoolevent.h"

typedef struct Tpool_attr{
	int maxsize;
	int minsize;
	int defsize;

	int Hload;
	int Lload;
	
	int autoflag;
	int frequency;
	int overtime;
}TPOOLATTR;

typedef struct Tpool_command{
	int command;
	void* iargs;
	void* oargs;
	int result;
}TPOOLCOMM;

typedef struct Tpool{
	int cursize;
	int available;
	int rate;
	int loadsilver;
	int status;

	TPOOLATTR attr;
	TPOOLCOMM command;
	TPOOLEQ eventQ;

	pthread_mutex_t lock;
	pthread_rwlock_t rwlock;

	pthread_mutex_t olock;
	pthread_cond_t ocond;

	pthread_mutex_t ilock;
	pthread_cond_t icond;

	TWORKER* workerlist;
}TPOOL;

//log
#define THREAD "thread"

//attr
#define TPOOLDEFMAXSIZE 128
#define TPOOLDEFMINSIZE 1
#define TPOOLDEFSIZE 4
#define TPOOLDEFHLOADRATE	80
#define TPOOLDEFLLOADRATE	20
#define TPOOLDEFAUTOFLAG 1
#define TPOOLDEFFREQUENCY	5
#define TPOOLDEFOVERTIME 30

//status
#define TPOOL_INIT 0
#define TPOOL_READY 1
#define TPOOL_UNAVAILABLE	2
#define TPOOL_ERROR 3

//commond
#define TPOOL_NONE 0
#define TPOOL_DESTROY	1
#define TPOOL_SIZECTL	2

//result
#define TPOOL_RETNONE -1
#define TPOOL_RETSUCC 0
#define TPOOL_RETERR 1

void Tpool_threadclean(void* );
void *Tpool_task(void*);
void *Tpool_monitor(void* );
int Tpool_createworker(TPOOL* ,int );
int Tpool_cancelworker(TPOOL* ,TWORKER* );

#endif
