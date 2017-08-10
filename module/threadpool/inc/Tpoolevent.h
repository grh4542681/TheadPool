#ifndef __TPOOLEVENT_H__
#define __TPOOLEVENT_H__

#include <pthread.h>
#include "eventlist.h"

typedef struct Tpool_eventQ{
	int num;
	int status;
	pthread_rwlock_t rwlock;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	ETASK* elist;
}TPOOLEQ;

#define TPOOL_EQINIT	0
#define TPOOL_EQREADY	1
#define TPOLL_EQUNAVAILABLE	2

void Tpool_evtQinit(TPOOLEQ* );
int Tpool_addevt(TPOOLEQ* ,ETASK* );
ETASK* Tpool_popevt(TPOOLEQ* );
void Tpool_clrevt(TPOOLEQ* );


#endif
