#ifndef __TPOOLTIMER_H__
#define __TPOOLTIMER_H__

typedef struct Tpool_timer{
	int status;
	pthread_rwlock_t rwlock;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	TTIMERLIST* timerlist;
}TTIMER;


#endif
