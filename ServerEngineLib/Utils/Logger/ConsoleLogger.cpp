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

    // 콘솔이 없는 경우 GetConsoleScreenBufferInfo가 실패할 수 있음
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (consoleHandle_ != INVALID_HANDLE_VALUE &&
        consoleHandle_ != nullptr &&
        GetConsoleScreenBufferInfo(consoleHandle_, &info))
    {
        defaultColor_ = info.wAttributes;
    }
    else
    {
        defaultColor_ = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }
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
    // 멀티스레드 로그 섞임 방지(필요 없으면 제거 가능)
    std::lock_guard<std::mutex> lock(logMutex_);

    // vswprintf_s 고정 1024는 길면 잘릴 수 있으니 동적 버퍼로
    // 1) 필요한 길이 계산
    int required = _vscwprintf(fmt, args);
    if (required <= 0) return;

    std::vector<WCHAR> buffer;
    buffer.resize(static_cast<size_t>(required) + 1);

    // va_list는 한 번 소비되면 재사용 불가라서 복사
    va_list argsCopy;
    va_copy(argsCopy, args);
    vswprintf_s(buffer.data(), buffer.size(), fmt, argsCopy);
    va_end(argsCopy);

    const WCHAR* text = buffer.data();
    DWORD len = static_cast<DWORD>(wcslen(text));

    // 1) 디버거 출력 (Rider/LLDB)
    if (debugOutputEnabled_) {
        OutputDebugStringW(text);
    }

    // 2) 콘솔/표준출력 출력
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (out == nullptr || out == INVALID_HANDLE_VALUE) {
        // 표준 출력 핸들이 없다면 콘솔 출력 불가
        return;
    }

    bool needColorChange = (Color::UseDefault != color);

    // 콘솔에 연결된 핸들이면 색상 적용 + WriteConsoleW
    if (IsConsoleHandle(out)) {
        if (needColorChange) SetColor(color);

        DWORD written = 0;
        WriteConsoleW(out, text, len, &written, nullptr);

        if (needColorChange) ResetColor();
    }
    else
    {
        // 리다이렉션(파일/파이프) 상태면 WriteConsoleW가 실패할 수 있으니
        // UTF-16을 그대로 WriteFile로 쓸 수도 있지만, 일반적으로는 UTF-8로 변환해서 쓰는 게 낫다.
        // 여기서는 간단히 UTF-8로 변환하여 출력.
        int utf8Len = WideCharToMultiByte(CP_UTF8, 0, text, (int)len, nullptr, 0, nullptr, nullptr);
        if (utf8Len > 0)
        {
            std::vector<char> utf8;
            utf8.resize((size_t)utf8Len);
            WideCharToMultiByte(CP_UTF8, 0, text, (int)len, utf8.data(), utf8Len, nullptr, nullptr);

            DWORD written = 0;
            WriteFile(out, utf8.data(), (DWORD)utf8.size(), &written, nullptr);
        }
    }
}

void ConsoleLogger::SetColor(Color color)
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (out && out != INVALID_HANDLE_VALUE && IsConsoleHandle(out))
        SetConsoleTextAttribute(out, static_cast<WORD>(color));
}

void ConsoleLogger::ResetColor()
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (out && out != INVALID_HANDLE_VALUE && IsConsoleHandle(out))
        SetConsoleTextAttribute(out, defaultColor_);
}

bool ConsoleLogger::IsConsoleHandle(HANDLE h)
{
    if (h == nullptr || h == INVALID_HANDLE_VALUE) return false;

    DWORD mode = 0;
    // 콘솔 핸들이면 GetConsoleMode가 성공함
    return GetConsoleMode(h, &mode) != 0;
}