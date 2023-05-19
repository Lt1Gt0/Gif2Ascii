#pragma once
#ifndef _ERROR_HPP_
#define _ERROR_HPP_

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include "utils/logger.hpp"
#define OPT [[maybe_unused]]

enum class Severity {
    low = 0,
    medium,
    high
};

inline void error(Severity severity)
{
    logger.Log(ERROR, "Exiting with severity: %d", (int)severity);
    std::cerr << '\n';
    std::exit((int)severity);
}

template<typename T, typename... Ts>
inline constexpr void error(Severity severity, T head, Ts... tail)
{
    std::cerr << head << " ";
    error(severity, tail...);
}

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
}

#endif // _ERROR_HPP_
