#pragma once

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

/* ======================
	C++ Standard Library
   ====================== */

#include <Windows.h>
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

/* ===================
    Windows / WinSock
   =================== */

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

/* ===========================
	Platform-dependent Engine
   =========================== */

#include "Network/SocketUtils.h"
