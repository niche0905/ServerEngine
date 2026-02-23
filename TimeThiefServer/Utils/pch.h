#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef _DEBUG
#pragma comment(lib, "ServerEngineLib\\Debug\\ServerEngineLib.lib")
#else
#pragma comment(lib, "ServerEngineLib\\Release\\ServerEngineLib.lib")
#endif

#include "Core/PCH/CorePch.h"
#include "Core/Global/CoreGlobal.h"
#include "Utils/Logger/ConsoleLogger.h"

// TODO: 다형성을 위해 수정이 필요하다
using ServiceRef = std::shared_ptr<class IocpServerService>;
