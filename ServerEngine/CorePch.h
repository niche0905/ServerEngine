#pragma once

// TODO: 필요한 것들 추가
#include "Types.h"
#include "Container.h"
#include "ThreadLocalStorage.h"
#include "SocketUtils.h"

#include <Windows.h>
#include <iostream>
#include <cassert>
#include <mutex>
#include <atomic>
#include <functional>

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
