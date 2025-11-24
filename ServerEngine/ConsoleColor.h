#pragma once
#include <windows.h>

/*---------
   Color
---------*/
//
// Color는 콘솔 출력 색상을 나타냅니다
//

enum class Color : WORD
{
	UseDefault = 0xFFFF,																	// Use default console color (no change)

	Default = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,							// Gray
	Red = FOREGROUND_RED | FOREGROUND_INTENSITY,											// Red
	Green = FOREGROUND_GREEN | FOREGROUND_INTENSITY,										// Green
	Blue = FOREGROUND_BLUE | FOREGROUND_INTENSITY,											// Blue
	Yellow = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,						// Yellow
	Cyan = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,						// Cyan
	Magenta = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,						// Magenta
	White = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,		// White
};
