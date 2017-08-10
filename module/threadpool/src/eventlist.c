#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "eventlist.h"
#include "mem.h"
#include "returnval.h"

void EventListCreate(ETASK* head)
{
	memset(head, 0x00, sizeof(ETASK));
}

int EventListDestroy(ETASK* head)
{
	ETASK* tmp = head;
	ETASK* tmp2 = NULL;
	int freenum = 0;
	if(head)
	{
		head->prev->next = NULL;
	}
	else
	{
		return 0;
	}
	while(tmp)
	{
		tmp2 = tmp;
		tmp = tmp->next;
		Free(tmp2);
		tmp2 = NULL;
		freenum++;
	}
	return freenum;
}

void EventListADD(ETASK** head,ETASK* etask)
{
	ETASK* task = (ETASK*)Malloc(sizeof(ETASK));
	memset(task,0x00,sizeof(ETASK));
	memcpy(task,etask,sizeof(ETASK));

	if(!(*head))
	{
		*head = task;
		(*head)->next = (*head);
		(*head)->prev = (*head);
	}
	else
	{
		ETASK* tmp = *head;
		do{
			tmp = tmp->prev;
			if(tmp->priority <= task->priority)
			{
				tmp->next->prev = task;
				task->next = tmp->next;
				tmp->next = task;
				task->prev = tmp;
				tmp=NULL;
				break;
			}
		}while(tmp != *head);
		if(tmp == (*head))
		{
			tmp->prev->next = task;
			task->prev = tmp->prev;
			tmp->prev = task;
			task->next = tmp;

			*head=task;
		}
	}
}

ETASK* EventListPop(ETASK** head)
{
	if(!(*head))
	{
		return NULL;
	}
	if((*head)->next == *head)
	{
		ETASK* tmp = *head;
		*head = NULL;
		return tmp;
	}
	else
	{
		ETASK* tmp = *head;
		(*head)->next->prev = (*head)->prev;
		(*head)->prev->next = (*head)->next;
		*head = (*head)->next;
		return tmp;
	}
}
