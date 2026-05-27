#pragma once
#include "Network/Buffer/RioBuffer/RioBufferPool.h"
#include "Utils/Types.h"

/*-----------------
   ThreadContext
-----------------*/
//
// ThreadContext는 thread 별로 관리되는 컨텍스트입니다
// TLS와 연동되어 각 thread가 자신의 Context(상태)를 가질 수 있도록 합니다
//

struct ThreadContext
{
	uint32 thread_id;		// thread 고유 id
	
#ifdef USE_RIO
	RioBufferPool rioBufferPool;
#endif

	// TODO: thread 별로 관리할 상태들 추가
};

