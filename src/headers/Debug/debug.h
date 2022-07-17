#pragma once
#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdarg.h>

// Not really the best debug methods for printing
// might rework later
namespace Debug
{
    void Print(const char* fmt, ...);
    void PrintErr(const char* fmt, ...);
}

#endif // _DEBUG_H
