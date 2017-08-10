#ifndef __RLIST_H__
#define __RLIST_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/types.h>


#define uoffsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define ucontainer_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - uoffsetof(type,member) );})

/**
 * 链表节点，包含一个前指针和一个后指针
 */
typedef struct RList{
	struct RList* next;
	struct RList* prev;
}RList;

/**
 * 元节点，包含一个链表节点和一个数据节点
 */
typedef struct DList{
	void* data;
	struct RList list;
}DList;



#define RLIST_HEAD(name) \
	struct RList name = LIST_HEAD_INIT(name)

#define RLIST_HEAD_INIT(name) { &(name), &(name) }

#define DList_entry(ptr, type, member) \
	ucontainer_of(ptr, type, member)

#endif