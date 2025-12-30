#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef _DEBUG
#pragma comment(lib, "ServerEngine\\Debug\\ServerEngine.lib")
#else
#pragma comment(lib, "ServerEngine\\Release\\ServerEngine.lib")
#endif

#include "Core/PCH/CorePch.h"

// TODO: 다형성을 위해 수정이 필요하다
using ServiceRef = std::shared_ptr<class IocpServerService>;
