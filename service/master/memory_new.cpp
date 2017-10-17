#include "stdafx.h"
#include "libBaseCommon/memory_hook.h"

#ifdef __MEMORY_HOOK__

void* operator new (size_t size)
{
	return base::alloc_hook(size, ((void**)&size)[-1]);
}

void* operator new[](size_t size)
{
	return base::alloc_hook(size, ((void**)&size)[-1]);
}

void operator delete (void* pData)
{
	base::dealloc_hook(pData);
}

void operator delete[](void* pData)
{
	base::dealloc_hook(pData);
}

#endif