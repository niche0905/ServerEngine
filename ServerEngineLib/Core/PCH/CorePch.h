#pragma once

/* ===================
	Windows / WinSock
   =================== */

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")

/* ======================
	C++ Standard Library
   ====================== */

#include <iostream>
#include <string_view>
#include <cassert>
#include <cmath>
#include <mutex>
#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <chrono>
#include <algorithm>


/* =============
	Engine Core
   ============= */

#include "Utils/Types.h"
#include "Utils/Container.h"
#include "Math/Math.h"
#include "Math/Vector.h"
#include "Core/Thread/ThreadLocalStorage.h"
#include "Core/Global/CoreGlobal.h"
#include "Core/Core.h"

/* ===========================
	Platform-dependent Engine
   =========================== */

#include "Network/SocketUtils.h"
