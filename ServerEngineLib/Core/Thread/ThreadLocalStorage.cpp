#include "pch.h"
#include "ThreadLocalStorage.h"


ThreadContext& TLS()
{
	thread_local static ThreadContext context;
	return context;
}