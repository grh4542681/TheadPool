#include "rlist.h"


/**
 * 通过数据区指针获取链表指针
 * @param data [数据区指针]
 * @param list [链表指针]
 */
void RList_entry_by_date(void** data,RList** list)
{
	DList* dlist=DList_entry(data,typeof(*dlist),data);
	*list=&(dlist->list);
}

/**
 * 通过链表指针获取数据区指针
 * @param list [链表指针]
 * @param data [数据区指针]
 */
void RData_entry_by_list(RList* list,void** data)
{
	DList* dlist=DList_entry(list,typeof(*dlist),list);
	*data=dlist->data;
}

/**
 * 添加链表节点
 * @param new  [新链表节点]
 * @param prev [插入位置链表节点前指针]
 * @param next [插入位置链表节点后指针]
 */
void __Rlist_add(struct RList *new,struct RList *prev,struct RList *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * 删除链表节点
 * @param prev [删除节点链表节点前指针]
 * @param next [删除节点链表节点后指针]
 */
void __Rlist_del(struct RList * prev, struct RList * next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * 初始化链表节点
 * @param list [链表节点指针]
 */
void INIT_LIST_HEAD(struct RList *list)
{
	list->next = list;
	list->prev = list;
}

/**
 * 通过数据节点创建包含链表节点的元节点
 * @param Dlist [元节点指针地址]
 * @param data  [数据节点指针]
 */
void Build_Dlist(DList** Dlist,void* data)
{
	DList* dlist = (DList*)malloc(sizeof(DList));
	memset(dlist, 0,sizeof(DList));
	INIT_LIST_HEAD(&(dlist->list));

	dlist->data = malloc(sizeof(*data));
	memcpy(dlist->data, data,sizeof(*data));

	*Dlist=dlist;
}

/**
 * 已知数据节点，向当前位置前插入链表
 * @param head [当前位置链表节点指针]
 * @param data [数据节点指针]
 */
void RList_add_head(struct RList* head,void* data)
{
	DList* dlist = (DList*)malloc(sizeof(DList));
	memset(dlist, 0,sizeof(DList));
	INIT_LIST_HEAD(&(dlist->list));

	dlist->data = malloc(sizeof(*data));
	memcpy(dlist->data, data,sizeof(*data));

	__Rlist_add(&(dlist->list),head,head->next);
}

/**
 * 已知数据节点，向当前位置后插入链表
 * @param head [当前位置链表节点指针]
 * @param data [数据节点指针]
 */
void RList_add_tail(struct RList* head,void* data)
{
	DList* dlist = (DList*)malloc(sizeof(DList));
	memset(dlist, 0,sizeof(DList));

	dlist->data = malloc(sizeof(*data));
	memcpy(dlist->data, data,sizeof(*data));

	__Rlist_add(&(dlist->list),head->prev,head);
}

/**
 * 删除链表节点，并释放整个元节点
 * @param head [删除链表节点指针]
 */
void RList_del(struct RList* head)
{
	__Rlist_del(head->prev,head->next);

	DList* entry=DList_entry(head,typeof(*entry),list);
	free(entry->data);
	free(entry);
}

/**
 * 替换旧节点，但旧节点不释放
 * @param old [旧链表节点指针]
 * @param new [新链表节点指针]
 */
void Rlist_replace(struct RList *old,struct RList *new)
{
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
	INIT_LIST_HEAD(old);
}

/**
 * 替换旧节点，旧节点释放
 * @param old [旧链表节点指针]
 * @param new [新链表节点指针]
 */
void Rlist_replace_del(struct RList *old,struct RList *new)
{
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;

	INIT_LIST_HEAD(old);
	RList_del(old);
}

/**
 * 移动节点到目标节点前
 * @param list [移动链表节点指针]
 * @param head [目标位置节点指针]
 */
void Rlist_move_head(struct RList *list, struct RList *head)
{
	__Rlist_del(list->prev,list->next);
	__Rlist_add(list,head,head->next);
}

/**
 * 移动节点到目标节点后
 * @param list [移动链表节点指针]
 * @param head [目标位置节点指针]
 */
void Rlist_move_tail(struct RList *list,struct RList *head)
{
	__Rlist_del(list->prev,list->next);
	__Rlist_add(list,head->prev,head);
}

/**
 * 目标节点是否为最后一个节点
 * @param  list [目标链表节点指针]
 * @param  head [链表头节点指针]
 * @return      [true or false]
 */
int Rlist_is_last(const struct RList *list,const struct RList *head)
{
	return list->next == head;
}

/**
 * 链表是否为空
 * @param  head [链表头节点指针]
 * @return      [true or false]
 */
int Rlist_empty(const struct RList *head)
{
	struct RList *next = head->next;
	return (next == head) && (next == head->prev);
}

/**
 * 合并链表
 * @param list [链表1合并位置链表节点指针]
 * @param prev [链表2合并位置链表节点前指针]
 * @param next [链表2合并位置链表节点后指针]
 */
void __Rlist_splice(const struct RList *list,struct RList *prev,struct RList *next)
{
	struct RList *first = list->next;
	struct RList *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/**
 * 合并链表到目标节点前
 * @param list [链表1合并位置链表节点指针]
 * @param head [链表2合并位置链表节点指针]
 */
void Rlist_splice_head(struct RList *list,struct RList *head)
{
	if (!Rlist_empty(list))
	{
		__Rlist_splice(list, head, head->next);
		INIT_LIST_HEAD(list);
	}
}

/**
 * 合并链表到目标节点后
 * @param list [链表1合并位置链表节点指针]
 * @param head [链表2合并位置链表节点指针]
 */
void Rlist_splice_tail(struct RList *list,struct RList *head)
{
	if (!Rlist_empty(list))
	{
		__Rlist_splice(list, head->prev, head);
		INIT_LIST_HEAD(list);
	}
}
