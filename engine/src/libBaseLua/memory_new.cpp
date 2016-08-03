#include "libBaseCommon/memory_hook.h"

#if defined __MEMORY_HOOK__ && defined _WIN32

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