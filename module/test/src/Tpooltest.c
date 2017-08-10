#include "log.h"
#include "Tpool.h"
#include "eventlist.h"
#include <unistd.h>
#include <string.h>

void* event(void* arg)
{
	APPLOG(THREAD,INFO,"%s","get event!!!!!!!!!!!!!!!!!!!!!!!!!");
	while(1){
	sleep(10);
	}
	return NULL;
}

int main(int argv,char** argc)
{
	int ret;
	LOGINIT("/home/helsinki/fram/etc/log.conf");

	APPLOG(THREAD,INFO,"%s","-------------------------Begin thread pool test!!!!!---------------------");


	TPOOL pool;
	memset(&pool,0x00,sizeof(TPOOL));
	sleep(10);

	ret = Tpool_create(&pool);
	printf("%d\n",ret);
	APPLOG(THREAD,INFO,"cursize[%d],available[%d],maxsize[%d],minsize[%d],status[%d]",pool.cursize,pool.available,pool.attr.maxsize,pool.attr.minsize,pool.status);
	sleep(5);
		Tpool_AddEvt(&pool,event,NULL,0);
		Tpool_AddEvt(&pool,event,NULL,0);

	Tpool_sizectl(&pool,-1);
	APPLOG(THREAD,INFO,"cursize[%d],available[%d],maxsize[%d],minsize[%d],status[%d]",pool.cursize,pool.available,pool.attr.maxsize,pool.attr.minsize,pool.status);
	sleep(5);

	Tpool_sizectl(&pool,10);
	APPLOG(THREAD,INFO,"cursize[%d],available[%d],maxsize[%d],minsize[%d],status[%d],rate[%d]",pool.cursize,pool.available,pool.attr.maxsize,pool.attr.minsize,pool.status,pool.rate);

		Tpool_AddEvt(&pool,event,NULL,0);
		Tpool_AddEvt(&pool,event,NULL,0);
	APPLOG(THREAD,INFO,"cursize[%d],available[%d],maxsize[%d],minsize[%d],status[%d],rate[%d]",pool.cursize,pool.available,pool.attr.maxsize,pool.attr.minsize,pool.status,pool.rate);

	printf("---------------------------------\n");

	Tpool_sizectl(&pool,-1);
	APPLOG(THREAD,INFO,"cursize[%d],available[%d],maxsize[%d],minsize[%d],status[%d]",pool.cursize,pool.available,pool.attr.maxsize,pool.attr.minsize,pool.status);
	sleep(10);
	
	Tpool_destroy(&pool);
	APPLOG(THREAD,INFO,"cursize[%d],available[%d],maxsize[%d],minsize[%d],status[%d]",pool.cursize,pool.available,pool.attr.maxsize,pool.attr.minsize,pool.status);
		Tpool_AddEvt(&pool,event,NULL,0);
		Tpool_AddEvt(&pool,event,NULL,0);
		Tpool_AddEvt(&pool,event,NULL,0);
		Tpool_AddEvt(&pool,event,NULL,0);
		Tpool_AddEvt(&pool,event,NULL,0);
		Tpool_AddEvt(&pool,event,NULL,0);
		Tpool_AddEvt(&pool,event,NULL,0);
		Tpool_AddEvt(&pool,event,NULL,0);
	Tpool_sizectl(&pool,-1);
		Tpool_AddEvt(&pool,event,NULL,0);

	while(1)
	{
	APPLOG(THREAD,INFO,"cursize[%d],available[%d],maxsize[%d],minsize[%d],status[%d],rate[%d]",pool.cursize,pool.available,pool.attr.maxsize,pool.attr.minsize,pool.status,pool.rate);
		sleep(1);
	}
/*

	ETASK* head=NULL;
	ETASK ent;
	memset(&ent,0x00,sizeof(ETASK));

	ent.priority = 100;
	EventListADD(&head,&ent);
	printf("%d\n",head->priority);
	printf("****************\n");
	ent.priority = 1;
	EventListADD(&head,&ent);
	printf("%d\n",head->priority);
	printf("%d\n",head->next->priority);
	printf("****************\n");
	ent.priority = 105;
	EventListADD(&head,&ent);
	printf("%d\n",head->priority);
	printf("%d\n",head->next->priority);
	printf("%d\n",head->next->next->priority);
	printf("****************\n");
	ent.priority = -101;
	EventListADD(&head,&ent);
	printf("%d\n",head->priority);
	printf("%d\n",head->next->priority);
	printf("%d\n",head->next->next->priority);
	printf("%d\n",head->next->next->next->priority);
	printf("****************\n");
	ent.priority = 9;
	EventListADD(&head,&ent);
	printf("%d\n",head->priority);
	printf("%d\n",head->next->priority);
	printf("%d\n",head->next->next->priority);
	printf("%d\n",head->next->next->next->priority);
	printf("%d\n",head->next->next->next->next->priority);
	printf("****************\n");
	
	ETASK* task1=NULL;
	task1=EventListPop(&head);
	printf("-----%d\n",task1->priority);
	task1=EventListPop(&head);
	printf("-----%d\n",task1->priority);
	task1=EventListPop(&head);
	printf("-----%d\n",task1->priority);

	ret=EventListDestroy(head);
	printf("ret=%d\n",ret);
*/
	LOGFREE();
	return 0;
}
