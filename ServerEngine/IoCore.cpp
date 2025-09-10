#include "pch.h"
#include "IoCore.h"

/*-----------
   IoCore
-----------*/

IoCore::IoCore()
{
	bool initSucc = Initialize();
	assert(initSucc && "IoCore Initialize Failed");
}

IoCore::~IoCore()
{
	Terminate();
}
