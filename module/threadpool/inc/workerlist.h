#ifndef __WORKERLIST_H__
#define __WORKERLIST_H__

#include <pthread.h>

typedef struct TWorker{
	pthread_t threadid;
	int role;
	int status;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	pthread_rwlock_t rwlock;
	struct TWorker* next;
}TWORKER;

#define MONITOR 1
#define WORKER	2

#define WORKERINIT 5
#define WORKERREADY	1
#define WORKERERROR	2
#define WORKERBUSY	3
#define WORKERDEAD 4
#define WORKERBEFREE 6

void WorkerListCreate(TWORKER* );
int WorkerListDestroy(TWORKER* );
int WorkerListADD(TWORKER** ,TWORKER* );
int WorkerListInsert(TWORKER** ,TWORKER* );
int WorkerListDEL(TWORKER** ,pthread_t );
TWORKER* GetWorkerById(TWORKER* ,pthread_t);

#endif
