#pragma once
#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdarg.h>

namespace Debug
{
    void Print(const char* fmt, ...);
    void PrintErr(const char* fmt, ...);
}

#endif // _DEBUG_H