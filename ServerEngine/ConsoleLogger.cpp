#include "pch.h"
#include "ConsoleLogger.h"
#include <cstdarg>
#include <cstdio>

/*-----------------
   ConsoleLogger
-----------------*/

ConsoleLogger::ConsoleLogger()
{
    consoleHandle_ = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO info{};
    GetConsoleScreenBufferInfo(consoleHandle_, &info);
    defaultColor_ = info.wAttributes;
}

void ConsoleLogger::Log(const WCHAR* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    InternalLog(Color::UseDefault, fmt, args);
    va_end(args);
}

void ConsoleLogger::Log(Color color, const WCHAR* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    InternalLog(color, fmt, args);
    va_end(args);
}

void ConsoleLogger::InternalLog(Color color, const WCHAR* fmt, va_list args)
{
    const size_t BUFFER_SIZE = 1024;
    WCHAR buffer[BUFFER_SIZE];

    // 포맷 문자열 처리
    vswprintf_s(buffer, BUFFER_SIZE, fmt, args);

    bool needColorChange = (Color::UseDefault != color);
    // 색상 적용
    if (needColorChange) {
        SetColor(color);
    }

    // 콘솔 출력
    DWORD written = 0;
    WriteConsoleW(consoleHandle_, buffer, (DWORD)wcslen(buffer), &written, nullptr);

    // 색상 리셋
    if (needColorChange) {
        ResetColor();
    }
}

void ConsoleLogger::SetColor(Color color)
{
    SetConsoleTextAttribute(consoleHandle_, static_cast<WORD>(color));
}

void ConsoleLogger::ResetColor()
{
    SetConsoleTextAttribute(consoleHandle_, defaultColor_);
}