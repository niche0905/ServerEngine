#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef _DEBUG
#pragma comment(lib, "ServerEngine\\Debug\\ServerEngine.lib")
#else
#pragma comment(lib, "ServerEngine\\Release\\ServerEngine.lib")
#endif

#include "CorePch.h"

using ServiceRef = std::shared_ptr<class IocpServerService>;
