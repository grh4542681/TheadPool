#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#include "log.h"
#include "mem.h"
#include "returnval.h"

#include "Tpoolcore.h"
#include "Tpoolevent.h"

/**
 * [Tpool_threadclean After the thread is finished clean up]
 * @param pool [Tpool pointer]
 */
void Tpool_threadclean(void* pool)
{
	//thread exit resource clean up
	pthread_t selfid = pthread_self();
	APPLOG(THREAD,INFO,"Thread[%ld] in cleanup program",selfid);
	pthread_mutex_unlock(&(((TPOOL*)pool)->eventQ.lock));

	TWORKER* selfworker = NULL;
	pthread_rwlock_rdlock(&(((TPOOL*)pool)->rwlock));
	selfworker = GetWorkerById(((TPOOL*)pool)->workerlist,selfid);
	pthread_rwlock_unlock(&(((TPOOL*)pool)->rwlock));
	if(!selfworker)
	{
		APPLOG(THREAD,ERROR,"Thread[%ld] not found in workerlist",selfid);
		return;
	}
	pthread_mutex_lock(&(selfworker->lock));
	selfworker->status = WORKERDEAD;
	pthread_cond_signal(&(selfworker->cond));
	pthread_mutex_unlock(&(selfworker->lock));
}

/**
 * [Tpool_task Work thread theme]
 * @param  pool [Tpool pointer]
 * @return      [description]
 */
void *Tpool_task(void* pool)
{
	pthread_t selfid = pthread_self();
	//thread detach
	pthread_detach(selfid);
	//push thread cleanup func
	pthread_cleanup_push(Tpool_threadclean,pool);

	TWORKER* selfworker = NULL;
	pthread_rwlock_rdlock(&(((TPOOL*)pool)->rwlock));
	selfworker = GetWorkerById(((TPOOL*)pool)->workerlist,selfid);
	pthread_rwlock_unlock(&(((TPOOL*)pool)->rwlock));
	if(!selfworker)
	{
		APPLOG(THREAD,ERROR,"Thread[%ld] not found in workerlist",selfid);
		pthread_exit((void*)-1);
	}
	//register to monitor
	APPLOG(THREAD,INFO,"Thread[%ld] is registeing for monitor",selfworker->threadid);
	pthread_mutex_lock(&(selfworker->lock));
	selfworker->status = WORKERREADY;
	pthread_cond_signal(&(selfworker->cond));
	pthread_mutex_unlock(&(selfworker->lock));

	//worker on event list
	ETASK* event = NULL;
	while(1)
	{
		
		event = NULL;
		APPLOG(THREAD,INFO,"Thread[%ld] is waiting for get event",selfworker->threadid);
		event = Tpool_popevt(&(((TPOOL*)pool)->eventQ));
		APPLOG(THREAD,INFO,"Thread[%ld] is getted event",selfworker->threadid);
		if(!event)
		{
			APPLOG(THREAD,ERROR,"Thread[%ld] pop event queue error",selfid)
			break;
		}
		pthread_rwlock_wrlock(&(((TPOOL*)pool)->rwlock));
		pthread_rwlock_wrlock(&(selfworker->rwlock));
		selfworker->status = WORKERBUSY;
		pthread_rwlock_unlock(&(selfworker->rwlock));
		(((TPOOL*)pool)->available)--;
		pthread_rwlock_unlock(&(((TPOOL*)pool)->rwlock));

		(*(event->function))(event->arg);

		pthread_rwlock_wrlock(&(((TPOOL*)pool)->rwlock));
		pthread_rwlock_wrlock(&(selfworker->rwlock));
		selfworker->status = WORKERREADY;
		pthread_rwlock_unlock(&(selfworker->rwlock));
		(((TPOOL*)pool)->available)++;
		pthread_rwlock_unlock(&(((TPOOL*)pool)->rwlock));
	
	//	sleep(5);
	}
	
	pthread_cleanup_pop(0);
	return NULL;
}

/**
 * [Tpool_createworker Create worker thread]
 * @param  pool [Tpool pointer]
 * @param  num  [number of thread]
 * @return      [SUCCESS or SYSERROR]
 */
int Tpool_createworker(TPOOL* pool,int num)
{
	int ret = 0;
	TWORKER* worker=NULL;
	int loop = 0;
	struct timeval now;
	struct timespec timeout;
	for(loop=num;loop>0;loop--)
	{
		worker = (TWORKER*)Malloc(sizeof(TWORKER));
		if(!worker)
		{
			APPLOG(THREAD,ERROR,"pthread_create errno[%d]:%s",errno,strerror(errno));
			return SYSERROR;

		}
		memset(worker,0x00,sizeof(TWORKER));
		worker->role = WORKER;
		worker->status = WORKERINIT;
		pthread_mutex_init(&(worker->lock),NULL);
		pthread_cond_init(&(worker->cond),NULL);
		pthread_rwlock_init(&(worker->rwlock),NULL);
		
		ret = pthread_create(&(worker->threadid),NULL,Tpool_task,pool);
		if(ret)
		{
			APPLOG(THREAD,ERROR,"pthread_create errno[%d]:%s",errno,strerror(errno));
			Free(worker);
			return SYSERROR;
		}

		pthread_rwlock_wrlock(&(pool->rwlock));
		ret = WorkerListInsert(&(pool->workerlist),worker);
		if(ret)
		{
			APPLOG(THREAD,ERROR,"WorkerListADD errno[%d]:%s",errno,strerror(errno));
			Free(worker);
			pthread_rwlock_unlock(&(pool->rwlock));
			return SYSERROR;
		}
		pthread_rwlock_unlock(&(pool->rwlock));

		// wait worker register
		memset(&now,0x00,sizeof(struct timeval));
		memset(&timeout,0x00,sizeof(struct timespec));

		gettimeofday(&now,NULL);
		timeout.tv_sec = now.tv_sec + pool->attr.overtime;
		timeout.tv_nsec = now.tv_usec * 1000;

		ret = 0;
		pthread_mutex_lock(&(worker->lock));
		while(ret!=ETIMEDOUT && worker->status==WORKERINIT)
		{
			ret = pthread_cond_timedwait(&(worker->cond),&(worker->lock),&timeout);
		}
		if(ret == ETIMEDOUT)
		{
			APPLOG(THREAD,ERROR,"Tpool worker[%ld] init timeout!",worker->threadid);
			worker->status = WORKERERROR;
			pthread_mutex_unlock(&(worker->lock));
			return SYSERROR;
		}
		else
		{
			if(worker->status != WORKERREADY)
			{
				APPLOG(THREAD,ERROR,"Tpool worker[%ld] init error!",worker->threadid);
				worker->status = WORKERERROR;
				pthread_mutex_unlock(&(worker->lock));
				return SYSERROR;
			}
			else
			{
				APPLOG(THREAD,INFO,"Worker[%ld] registered over",worker->threadid);
			}
		}
		pthread_mutex_unlock(&(worker->lock));
		
		//change pool arguments
		pthread_rwlock_wrlock(&(pool->rwlock));
		(pool->cursize)++;
		(pool->available)++;
		pthread_rwlock_unlock(&(pool->rwlock));

	}
	return SUCCESS;	
}

/**
 * [Tpool_cancelworker Cancel worker thread]
 * @param  pool   [Tpool pointer]
 * @param  worker [will be canceled worker struct pointer]
 * @return        [SUCCESS or SYSERROR]
 */
int Tpool_cancelworker(TPOOL* pool,TWORKER* worker)
{
	int ret = 0;
	struct timeval now;
	struct timespec timeout;

	memset(&now,0x00,sizeof(struct timeval));
	memset(&timeout,0x00,sizeof(struct timespec));

	
	pthread_rwlock_wrlock(&(worker->rwlock));
	pthread_rwlock_wrlock(&(pool->rwlock));
	if(worker->status == WORKERDEAD)
	{
		pthread_rwlock_unlock(&(pool->rwlock));
		pthread_rwlock_unlock(&(worker->rwlock));
		return SUCCESS;
	}
	else
	{
		(pool->cursize)--;
		if(worker->status == WORKERREADY||worker->status == WORKERBEFREE)
		{
			(pool->available)--;
		}
		pthread_rwlock_unlock(&(pool->rwlock));
		worker->status = WORKERERROR;
		pthread_rwlock_unlock(&(worker->rwlock));

		pthread_mutex_lock(&(worker->lock));
		pthread_cancel(worker->threadid);

		gettimeofday(&now,NULL);
		timeout.tv_sec = now.tv_sec + pool->attr.overtime;
		timeout.tv_nsec = now.tv_usec * 1000;
		ret = 0;
		while(ret!=ETIMEDOUT && worker->status!=WORKERDEAD)
		{
			ret = pthread_cond_timedwait(&(worker->cond),&(worker->lock),&timeout);
		}
		if(ret == ETIMEDOUT)
		{
			APPLOG(THREAD,ERROR,"Cancel thread[%ld] timeout",worker->threadid);
			pthread_mutex_unlock(&(worker->lock));
			return SYSERROR;
		}
		else
		{
			pthread_mutex_unlock(&(worker->lock));
			return SUCCESS;
		}
	}
} 

/**
 * [Tpool_workerclean Worker thread resources clean]
 * @param  worker [worker pointer]
 * @return        [description]
 */
void Tpool_workerclean(TWORKER* worker)
{
	pthread_mutex_destroy(&(worker->lock));
	pthread_cond_destroy(&(worker->cond));
	pthread_rwlock_destroy(&(worker->rwlock));
}
/**
 * [_Tpool_destroy Destroy the pool]
 * @param  pool [Tpool pointer]
 * @return      [SUCCESS or SYSERROR]
 */
int _Tpool_destroy(TPOOL* pool)
{
	//change pool status
	pthread_rwlock_wrlock(&(((TPOOL*)pool)->rwlock));
	((TPOOL*)pool)->status = TPOOL_UNAVAILABLE;
	pthread_rwlock_unlock(&(((TPOOL*)pool)->rwlock));

	//cancel all threads
	TWORKER* tmp = ((TPOOL*)pool)->workerlist->next;
	while(tmp)
	{
		Tpool_cancelworker((TPOOL*)pool,tmp);
		Tpool_workerclean(tmp);

		tmp=tmp->next;
	}
	
	//destroy worker lsit
	pthread_rwlock_wrlock(&(((TPOOL*)pool)->rwlock));
	WorkerListDestroy(((TPOOL*)pool)->workerlist);
	pthread_rwlock_unlock(&(((TPOOL*)pool)->rwlock));
	
	//destroy event list
	Tpool_clrevt(&(((TPOOL*)pool)->eventQ));

	return SUCCESS;
}

/**
 * [Tpool_clean Tpool resources clean]
 * @param  pool [Tpool pointer]
 * @return      [description]
 */
void Tpool_clean(TPOOL* pool)
{
	pthread_mutex_destroy(&(pool->lock));
	pthread_rwlock_destroy(&(pool->rwlock));

	pthread_mutex_destroy(&(pool->ilock));
	pthread_cond_destroy(&(pool->icond));

	pthread_mutex_destroy(&(pool->olock));
	pthread_cond_destroy(&(pool->ocond));
}

/**
 * [_Tpool_sizectl Pool size control]
 * @param  pool [Tpool pointer]
 * @param  size [>0 expand,<0 shrink]
 * @return      [SUCCESS or SYSERROR]
 */
int _Tpool_sizectl(TPOOL* pool,int size)
{
	if(size > 0)
	{
		//expand pool
		APPLOG(THREAD,INFO,"Expand pool size [%d]",size);
		if( (pool->cursize + size) > pool->attr.maxsize )
		{
			APPLOG(THREAD,ERROR,"Expand pool [%d] will exceeds the maximum[%d],expand error.",size,pool->attr.maxsize);
			return SYSERROR;
		}
		return Tpool_createworker(pool,size);
	}
	else if(size < 0)
	{
		//shrink pool
		APPLOG(THREAD,INFO,"Shrink pool size [%d]",size);
		if( pool->available < (0-size) )
		{
			APPLOG(THREAD,ERROR,"Shrink pool [%d] error,There are insufficient free threads available[%d]",size,pool->available);
			return SYSERROR;
		}
		int loop=0;
		TWORKER* tmp = pool->workerlist;
		while(tmp)
		{
			pthread_rwlock_wrlock(&(tmp->rwlock));
			if(tmp->status == WORKERREADY)
			{
				tmp->status = WORKERBEFREE;
				pthread_rwlock_unlock(&(tmp->rwlock));

				Tpool_cancelworker(pool,tmp);
				Tpool_workerclean(tmp);

				TWORKER* tmp1 = tmp;
				tmp = tmp->next;
				pthread_rwlock_wrlock(&(pool->rwlock));
				WorkerListDEL(&(pool->workerlist),tmp1->threadid);
				pthread_rwlock_unlock(&(pool->rwlock));
				loop++;
			}
			else
			{
				pthread_rwlock_unlock(&(tmp->rwlock));
				tmp = tmp->next;
			}
			if(loop==(0-size))
			{
				break;
			}
		}
		return loop;
	}
	else
	{
		APPLOG(THREAD,ERROR,"Unknow pool size agrs[%d]",size);
		return SYSERROR;
	}
}

int Tpool_loadcal(TPOOL* pool)
{
	pool->rate = (pool->cursize - pool->available)*100/(pool->cursize);
	if(pool->cursize == pool->attr.maxsize || pool->cursize == pool->attr.maxsize)
	{
		APPLOG(THREAD,INFO,"%s","Tpool already in max or min size");
		return 0;
	}
	if(pool->rate >= pool->attr.Hload)
	{
		if(pool->loadsilver <= 0)
		{
			pool->loadsilver = 1;
		}
		else
		{
			pool->loadsilver++;
		}
	}
	else if(pool->rate <= pool->attr.Lload)
	{
		if(pool->loadsilver >= 0)
		{
			pool->loadsilver = -1;
		}
		else
		{
			pool->loadsilver--;
		}
	}
	else
	{

	}
	
	if(pool->loadsilver > 5)
	{
		return ((pool->cursize)/5);
	}
	else if(pool->loadsilver < -5)
	{
		return 0-((pool->cursize)/5);
	}
	else
	{
		return 0;
	}

}

/**
 * [Tpool_monitor Monitor thread for pool]
 * @param  pool [Tpool pointer]
 * @return      [description]
 */
void *Tpool_monitor(void* pool)
{
	int ret = 0;
	int loadctl = 0;
	struct timeval now;
	struct timespec timeout;

	//thread detach
	pthread_detach(pthread_self());
	
	//Create min threads
	ret = Tpool_createworker((TPOOL*)pool,((TPOOL*)pool)->attr.defsize);
	if(ret)
	{
		APPLOG(THREAD,ERROR,"%s","Create worker at init pool error");
		pthread_mutex_lock(&(((TPOOL*)pool)->olock));
		((TPOOL*)pool)->status = TPOOL_UNAVAILABLE;
		pthread_cond_signal(&(((TPOOL*)pool)->ocond));
		pthread_mutex_unlock(&(((TPOOL*)pool)->olock));
		pthread_exit((void*)-1);
	}
	
	//notice main thread
	pthread_mutex_lock(&(((TPOOL*)pool)->olock));
	((TPOOL*)pool)->status = TPOOL_READY;
	pthread_cond_signal(&(((TPOOL*)pool)->ocond));
	pthread_mutex_unlock(&(((TPOOL*)pool)->olock));
	sleep(1); //for lock safe

	//Begin monitor
	pthread_mutex_lock(&(((TPOOL*)pool)->olock));
	while(1)
	{
		memset(&now,0x00,sizeof(struct timeval));
		memset(&timeout,0x00,sizeof(struct timespec));

		gettimeofday(&now,NULL);
		timeout.tv_sec = now.tv_sec + 5;
		timeout.tv_nsec = now.tv_usec * 1000;
		ret = 0;
		while(ret!=ETIMEDOUT && ((TPOOL*)pool)->command.command==TPOOL_NONE)
		{
			ret = pthread_cond_timedwait(&(((TPOOL*)pool)->ocond),&(((TPOOL*)pool)->olock),&timeout);
		}
		if(ret == ETIMEDOUT)
		{
			//Normal
			APPLOG(THREAD,INFO,"%s","Tpool load timing check ...");
			loadctl = Tpool_loadcal((TPOOL*)pool);
			if(loadctl > 0)
			{
				APPLOG(THREAD,INFO,"%s","Tpool Load is too high");
				if(((TPOOL*)pool)->cursize + loadctl > ((TPOOL*)pool)->attr.maxsize)
					loadctl = ((TPOOL*)pool)->attr.maxsize - ((TPOOL*)pool)->cursize;
				_Tpool_sizectl(pool,loadctl);	
				((TPOOL*)pool)->loadsilver = 0;
			}
			else if(loadctl < 0)
			{
				APPLOG(THREAD,INFO,"%s","Tpool Load is too low");
				if(((TPOOL*)pool)->cursize + loadctl < ((TPOOL*)pool)->attr.minsize)
					loadctl = ((TPOOL*)pool)->attr.minsize - ((TPOOL*)pool)->cursize;
				_Tpool_sizectl(pool,loadctl);	
				((TPOOL*)pool)->loadsilver = 0;
			}
			else
			{
				APPLOG(THREAD,INFO,"%s","Tpool is health");
			}
		}
		else
		{
			//Command
			if(((TPOOL*)pool)->command.command == TPOOL_DESTROY)
			{
				APPLOG(THREAD,INFO,"%s","Destroy tpool ...");
				ret = _Tpool_destroy((TPOOL*)pool);
				pthread_mutex_lock(&(((TPOOL*)pool)->ilock));
				if(ret == SYSERROR)
				{
					((TPOOL*)pool)->command.result=TPOOL_RETERR;
				}
				else
				{
					((TPOOL*)pool)->command.result=TPOOL_RETSUCC;
				}
				pthread_cond_signal(&(((TPOOL*)pool)->icond));
				pthread_mutex_unlock(&(((TPOOL*)pool)->ilock));
				
				((TPOOL*)pool)->command.command=TPOOL_NONE;
				//monitor exit
				pthread_exit((void*)0);
			}
			else if(((TPOOL*)pool)->command.command == TPOOL_SIZECTL)
			{
				APPLOG(THREAD,INFO,"Control tpool size for [%d]",*((int*)((TPOOL*)pool)->command.iargs));
				ret = _Tpool_sizectl(pool,*((int*)((TPOOL*)pool)->command.iargs));
				pthread_mutex_lock(&(((TPOOL*)pool)->ilock));
				if(ret == SYSERROR)
				{
					((TPOOL*)pool)->command.result=TPOOL_RETERR;
				}
				else
				{
					((TPOOL*)pool)->command.result=TPOOL_RETSUCC;
				}
				pthread_cond_signal(&(((TPOOL*)pool)->icond));
				pthread_mutex_unlock(&(((TPOOL*)pool)->ilock));

				((TPOOL*)pool)->command.command=TPOOL_NONE;
			}
			else
			{
				APPLOG(THREAD,ERROR,"Unknow thread pool commond[%d]",((TPOOL*)pool)->command.command);
				break;
			}

		}
	}
	pthread_mutex_unlock(&(((TPOOL*)pool)->olock));
	return NULL;
}





