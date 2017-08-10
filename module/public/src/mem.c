#include "mem.h"

void* Malloc(long length)
{
    return (void*)malloc(length);
}

void Free(void* ptr)
{
	free(ptr);
}
