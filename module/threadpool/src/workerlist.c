#include <string.h>

#include "workerlist.h"
#include "mem.h"
#include "returnval.h"

int WorkerListDestroy(TWORKER* head)
{
	TWORKER* tmp = head;
	TWORKER* tmp_next = NULL;
	int Delnum = 0;
	while(tmp)
	{
		tmp_next = tmp->next;
		Free(tmp);
		tmp = tmp_next;
		Delnum++;
	}
	return Delnum;
}

int WorkerListADD(TWORKER** head,TWORKER* worker)
{
	TWORKER* p = NULL;
	p = (TWORKER*)Malloc(sizeof(TWORKER));
	if(!p)
	{
		return SYSERROR;
	}
	memset(p,0x00,sizeof(TWORKER));

	memcpy(p,worker,sizeof(TWORKER));
	p->next = NULL;

	if(!(*head))
	{
		*head = p;
	}
	else
	{
		TWORKER* tmp = *head;
		while(tmp->next)
		{
			tmp = tmp->next;
		}
		tmp->next = p;
	}
	return SUCCESS;
}

int WorkerListInsert(TWORKER** head,TWORKER* worker)
{
	if(!(*head))
	{
		head = &worker;
	}
	else
	{
		TWORKER* tmp = *head;
		while(tmp->next)
		{
			tmp = tmp->next;
		}
		tmp->next = worker;
	}
	return SUCCESS;

}

int WorkerListDEL(TWORKER** head,pthread_t tid)
{
	TWORKER* tmp = *head;
	TWORKER* tmp_father = NULL;
	int Delnum = 0;
	while(tmp)
	{
		if(tmp->threadid == tid)
		{
			if(!tmp_father)
			{
				*head = tmp->next;
				Free(tmp);
				tmp = *head;
				Delnum++;
				continue;
			}
			else
			{
				tmp_father->next = tmp->next;
				Free(tmp);
				tmp = tmp_father->next;
				Delnum++;
				continue;
			}
		}
		else
		{
			tmp_father = tmp;
			tmp = tmp->next;
		}
	}
	return Delnum;
}

TWORKER* GetWorkerById(TWORKER* head,pthread_t tid)
{
	TWORKER* tmp = head;
	while(tmp)
	{
		if(tmp->threadid == tid)
		{
			return tmp;
		}
		tmp = tmp->next;
	}
	return NULL;	
}








