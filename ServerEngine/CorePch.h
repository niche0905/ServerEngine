#pragma once

/* =============
	Engine Core
   ============= */

#include "Types.h"
#include "Container.h"
#include "ThreadLocalStorage.h"

/* ======================
	C++ Standard Library
   ====================== */

#include <Windows.h>
#include <iostream>
#include <cassert>
#include <mutex>
#include <atomic>
#include <functional>

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

#include "SocketUtils.h"
