#include "Tpoolevent.h"
#include "Tpoolcore.h"
#include "returnval.h"
#include "log.h"

void Tpool_evtQinit(TPOOLEQ* eventQ)
{
	eventQ->num = 0;
	eventQ->status = TPOOL_EQINIT;
	eventQ->elist = NULL;
	pthread_rwlock_init(&(eventQ->rwlock),NULL);
	pthread_mutex_init(&(eventQ->lock),NULL);
	pthread_cond_init(&(eventQ->cond),NULL);

	eventQ->status = TPOOL_EQREADY;
}

int Tpool_addevt(TPOOLEQ* eventQ,ETASK* event)
{
	pthread_rwlock_rdlock(&(eventQ->rwlock));
	if(eventQ->status != TPOOL_EQREADY)
	{
		pthread_rwlock_unlock(&(eventQ->rwlock));
		APPLOG(THREAD,ERROR,"%s","Event queue not ready.");
		return SYSERROR;
	}
	else
		pthread_rwlock_unlock(&(eventQ->rwlock));

	pthread_mutex_lock(&(eventQ->lock));
	EventListADD(&(eventQ->elist),event);
	eventQ->num++;
	pthread_cond_signal(&(eventQ->cond));
	pthread_mutex_unlock(&(eventQ->lock));
	return SUCCESS;
}

ETASK* Tpool_popevt(TPOOLEQ* eventQ)
{
	pthread_rwlock_rdlock(&(eventQ->rwlock));
	if(eventQ->status != TPOOL_EQREADY)
	{
		pthread_rwlock_unlock(&(eventQ->rwlock));
		APPLOG(THREAD,ERROR,"%s","Event queue not ready.");
		return NULL;
	}
	else
		pthread_rwlock_unlock(&(eventQ->rwlock));

	ETASK* event = NULL;
	pthread_mutex_lock(&(eventQ->lock));
	while(eventQ->num == 0)
    {   
        pthread_cond_wait(&(eventQ->cond),&(eventQ->lock));
    }
	event = EventListPop(&(eventQ->elist));
	eventQ->num--;
	pthread_mutex_unlock(&(eventQ->lock));
	return event;
}

void Tpool_clrevt(TPOOLEQ* eventQ)
{
	pthread_rwlock_wrlock(&(eventQ->rwlock));
	if(eventQ->status != TPOOL_EQREADY)
	{
		pthread_rwlock_unlock(&(eventQ->rwlock));
		APPLOG(THREAD,ERROR,"%s","Event queue not ready.");
		return ;
	}
	else
	{
		eventQ->status = TPOLL_EQUNAVAILABLE;
		pthread_rwlock_unlock(&(eventQ->rwlock));
	}
	EventListDestroy(eventQ->elist);
	pthread_mutex_destroy(&(eventQ->lock));
	pthread_cond_destroy(&(eventQ->cond));
	pthread_rwlock_destroy(&(eventQ->rwlock));
}

