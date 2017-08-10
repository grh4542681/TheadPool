#ifndef __TASKLIST_H__
#define __TASKLIST_H__

typedef struct eventtask{
	void* (*function)(void *); 
	void *arg;
	int priority;
	int status;

	//int overtime
	//void (*overfunc)(void *);

	struct eventtask* next;
	struct eventtask* prev;
}ETASK;

int EventListDestroy(ETASK* );
void EventListADD(ETASK** ,ETASK* );
ETASK* EventListPop(ETASK** );

#define IGNORE 0

#endif
