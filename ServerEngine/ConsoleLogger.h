#pragma once
#include <string>
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

    // Default Color log
    void Log(const WCHAR* fmt, ...);

    // User Customize Color log
    void Log(Color color, const WCHAR* fmt, ...);

private:
    HANDLE consoleHandle_;
    WORD   defaultColor_;   // 콘솔 초기 색상 저장

private:
    void InternalLog(Color color, const WCHAR* fmt, va_list args);

    void SetColor(Color color);
    void ResetColor();
};
