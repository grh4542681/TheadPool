#ifndef __TPOOL_H__
#define __TPOOL_H__

#include "Tpoolcore.h"

void Tpool_destroy(TPOOL* );
int Tpool_create(TPOOL*);
int Tpool_sizectl(TPOOL* ,int );
int Tpool_AddEvt(TPOOL* ,void* (*)(void*),void* ,int );

#endif

