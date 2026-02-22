#pragma once
#include <string>
#include <mutex>
#include <windows.h>
#include "ConsoleColor.h"

/*-----------------
   ConsoleLogger
-----------------*/
//
// ConsoleLogger는 콘솔에 색상으로 로그를 출력합니다
// 
// 사용 예시
// logger.Log(Color::Green, L"[INFO] Server started. Port=%d\n", 7777);
// logger.Log(Color::Yellow, L"[WARN] Latency %.2f ms\n", 123.45);
//

class ConsoleLogger
{
public:
    ConsoleLogger();
    ~ConsoleLogger() = default;

    void Log(const WCHAR* fmt, ...);
    void Log(Color color, const WCHAR* fmt, ...);

    // 디버거 출력 On/Off
    void SetDebugOutputEnabled(bool enabled) { debugOutputEnabled_ = enabled; }

private:
    HANDLE consoleHandle_ = INVALID_HANDLE_VALUE;
    WORD   defaultColor_ = 0;
    bool   debugOutputEnabled_ = true;

    std::mutex logMutex_;

private:
    void InternalLog(Color color, const WCHAR* fmt, va_list args);

    void SetColor(Color color);
    void ResetColor();

    static bool IsConsoleHandle(HANDLE h);
    static void WriteToHandle(HANDLE h, const WCHAR* text, DWORD len);
};