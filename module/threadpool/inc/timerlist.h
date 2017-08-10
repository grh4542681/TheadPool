#ifndef __TIMERLIST_H__
#define __TIMERLIST_H__

#include <pthread.h>

typedef struct timerlist{
	pthread_t threadid;
	int status;
	int unit;
	int overtime;
	struct timerlist* next;
}TTIMER;


#endif
