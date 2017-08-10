#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#include "returnval.h"
#include "log.h"
#include "mem.h"

#include "Tpool.h"
#include "Tpoolcore.h"
#include "Tpoolevent.h"

/**
 * [Tpool_create Create a threadpool use argument]
 * @param  pool [Tpool struct pointer]
 * @return      [SUCCESS or SYSERROR]
 */
int Tpool_create(TPOOL* pool)
{
	int ret = 0;

	pool->status = TPOOL_INIT;
	pool->command.result = TPOOL_RETNONE;

	//Init attr
	if(pool->attr.Hload > 100 || pool->attr.Hload < 0 || pool->attr.Lload > 100 || pool->attr.Lload < 0)
	{
		APPLOG(THREAD,ERROR,"%s","Attr for pool has error value");
		return SYSERROR;
	}
	
	if(pool->attr.maxsize == 0)
	{
		pool->attr.maxsize = TPOOLDEFMAXSIZE;
	}
	if(pool->attr.minsize == 0)
	{
		pool->attr.minsize = TPOOLDEFMINSIZE;
	}
	if(pool->attr.defsize == 0)
	{
		pool->attr.defsize = TPOOLDEFSIZE;
	}
	if(pool->attr.Hload == 0)
	{
		pool->attr.Hload = TPOOLDEFHLOADRATE;
	}
	if(pool->attr.Lload == 0)
	{
		pool->attr.Lload = TPOOLDEFLLOADRATE;
	}
	if(pool->attr.autoflag == 0)
	{
		pool->attr.autoflag= TPOOLDEFAUTOFLAG;
	}
	if(pool->attr.frequency == 0)
	{
		pool->attr.frequency = TPOOLDEFFREQUENCY;
	}
	if(pool->attr.overtime == 0)
	{
		pool->attr.overtime = TPOOLDEFOVERTIME;
	}
	
	//Init mutex lock
	pthread_rwlock_init(&(pool->rwlock),NULL);
	pthread_mutex_init(&(pool->olock),NULL);
	pthread_cond_init(&(pool->ocond),NULL);
	pthread_mutex_init(&(pool->ilock),NULL);
	pthread_cond_init(&(pool->icond),NULL);

	//Init event queue
	Tpool_evtQinit(&(pool->eventQ));

	//Create monitor thread
	pool->workerlist = (TWORKER*)Malloc(sizeof(TWORKER));
	memset(pool->workerlist,0x00,sizeof(TWORKER));
	pool->workerlist->role = MONITOR;

	ret = pthread_create(&(pool->workerlist->threadid),NULL,Tpool_monitor,(void*)pool);
	if(ret)
	{
		APPLOG(THREAD,ERROR,"pthread_create errno[%d]:%s",errno,strerror(errno));
		Free(pool->workerlist);
		pool->workerlist = NULL;
		return SYSERROR;
	}
	
	//Wait monitor thread init the pool
	struct timeval now;
	struct timespec timeout;
	memset(&now,0x00,sizeof(struct timeval));
	memset(&timeout,0x00,sizeof(struct timespec));

	gettimeofday(&now,NULL);
	timeout.tv_sec = now.tv_sec + pool->attr.overtime;
	timeout.tv_nsec = now.tv_usec * 1000;

	ret = pthread_mutex_lock(&pool->olock);
	while(ret!=ETIMEDOUT && pool->status==TPOOL_INIT)
	{
		ret = pthread_cond_timedwait(&(pool->ocond),&(pool->olock),&timeout);
	}
	if(ret == ETIMEDOUT)
	{
		APPLOG(THREAD,ERROR,"%s","Tpool init timeout!");
		pthread_mutex_unlock(&pool->olock);
		WorkerListDestroy(pool->workerlist);
		pool->workerlist = NULL;
		return SYSERROR;
	}
	else
	{
		if(pool->status != TPOOL_READY)
		{
			APPLOG(THREAD,ERROR,"%s","Tpool init error!");
			pthread_mutex_unlock(&pool->olock);
			WorkerListDestroy(pool->workerlist);
			pool->workerlist = NULL;
			return SYSERROR;
		}
	}
	pthread_mutex_unlock(&pool->olock);
	APPLOG(THREAD,INFO,"%s","Create Tpool success");
	return SUCCESS;
}

/**
 * [Tpool_destroy Destroy a thread pool]
 * @param pool [Tpool struct pointer]
 */
void Tpool_destroy(TPOOL* pool)
{

	pthread_rwlock_rdlock(&(pool->rwlock));
	if(pool->status != TPOOL_READY)
	{
		pthread_rwlock_unlock(&(pool->rwlock));
		APPLOG(THREAD,ERROR,"%s","Tpool is not ready");
		return;
	}
	else
		pthread_rwlock_unlock(&(pool->rwlock));

	int ret=0;

	struct timeval now;
	struct timespec timeout;

	memset(&now,0x00,sizeof(struct timeval));
	memset(&timeout,0x00,sizeof(struct timespec));

	pthread_mutex_lock(&(pool->olock));
	pool->command.command=TPOOL_DESTROY;
	pthread_cond_signal(&(pool->ocond));
	pthread_mutex_unlock(&(pool->olock));

	gettimeofday(&now,NULL);
	timeout.tv_sec = now.tv_sec + pool->attr.overtime;
	timeout.tv_nsec = now.tv_usec * 1000;
	ret = 0;

	pthread_mutex_lock(&(pool->ilock));
	while(ret!=ETIMEDOUT && pool->command.result==TPOOL_RETNONE)
	{   
		ret = pthread_cond_timedwait(&(pool->icond),&(pool->ilock),&timeout);
	}   
	if(ret == ETIMEDOUT)
	{   
		APPLOG(THREAD,ERROR,"%s","Destoty Tpool timeout");
		pthread_mutex_unlock(&(pool->ilock));
	}
	else
	{
		pthread_mutex_unlock(&(pool->ilock));
	}
	APPLOG(THREAD,ERROR,"%s","Destoty Tpool success");
}

/**
 * [Tpool_sizectl Control the size of thread pool]
 * @param  pool [Tpool struct pointer]
 * @param  size [>0 extends <0 shink]
 * @return      [SUCCESS or SYSERROR]
 */
int Tpool_sizectl(TPOOL* pool,int size)
{
	pthread_rwlock_rdlock(&(pool->rwlock));
	if(pool->status != TPOOL_READY)
	{
		pthread_rwlock_unlock(&(pool->rwlock));
		APPLOG(THREAD,ERROR,"%s","Tpool is not ready");
		return SYSERROR;
	}
	else
		pthread_rwlock_unlock(&(pool->rwlock));

	int ret=0;

	struct timeval now;
	struct timespec timeout;

	memset(&now,0x00,sizeof(struct timeval));
	memset(&timeout,0x00,sizeof(struct timespec));

	int* psize = (int*)Malloc(sizeof(int));
	memset(psize,0x00,sizeof(int));
	memcpy(psize,&size,sizeof(int));

	pthread_mutex_lock(&(pool->olock));
	pool->command.command=TPOOL_SIZECTL;
	pool->command.iargs=(void*)psize;
	pthread_cond_signal(&(pool->ocond));
	pthread_mutex_unlock(&(pool->olock));

	gettimeofday(&now,NULL);
	timeout.tv_sec = now.tv_sec + pool->attr.overtime;
	timeout.tv_nsec = now.tv_usec * 1000;
	ret = 0;

	pthread_mutex_lock(&(pool->ilock));
	while(ret!=ETIMEDOUT && pool->command.result==TPOOL_RETNONE)
	{   
		ret = pthread_cond_timedwait(&(pool->icond),&(pool->ilock),&timeout);
	}   
	if(ret == ETIMEDOUT)
	{   
		APPLOG(THREAD,ERROR,"%s","Tpool sizectl timeout");
	}
	else
	{
		
	}
	pool->command.result=TPOOL_RETNONE;
	Free(psize);
	pthread_mutex_unlock(&(pool->ilock));
	
	return SUCCESS;
}

/**
 * [Tpool_AddEvt Add a event into Tpool]
 * @param  pool     		[Tpool struct pointer]
 * @param  function         [event function]
 * @param  arg     			[function arguments]
 * @param  priority 		[function execute priority]
 * @return          		[SUCCESS or SYSERROR]
 */
int Tpool_AddEvt(TPOOL* pool,void* (*function)(void*),void* arg,int priority)
{
	pthread_rwlock_rdlock(&(pool->rwlock));
	if(pool->status != TPOOL_READY)
	{
		pthread_rwlock_unlock(&(pool->rwlock));
		APPLOG(THREAD,ERROR,"%s","Tpool is not ready");
		return SYSERROR;
	}
	else
		pthread_rwlock_unlock(&(pool->rwlock));

	ETASK event;
	memset(&event,0x00,sizeof(ETASK));

	event.function=function;
	event.arg=arg;
	event.priority=priority;

	Tpool_addevt(&(pool->eventQ),&event);
	return SUCCESS;
}


