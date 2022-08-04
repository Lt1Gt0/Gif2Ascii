#pragma once
#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "Debug/logger.h"

enum class Severity {
    low = 0,
    medium,
    high
};

namespace Debug
{
    inline void Print(const char* fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stdout, fmt, ap);
        fprintf(stdout, "\n");
        fflush(stdout);
        va_end(ap);
    }
    
    inline void PrintErr(const char* fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
        fflush(stderr);
        va_end(ap);
    }


    inline void error(Severity severity)
    {
        LOG_ERROR << "Exiting with severity: " << (int)severity << std::endl;
        std::cerr << '\n';
        std::exit((int)severity);
    }

    template<typename T, typename... Ts>
    inline constexpr void error(Severity severity, T head, Ts... tail)
    {
        std::cerr << head << " ";
        error(severity, tail...);
    }
}

#endif // _DEBUG_H
