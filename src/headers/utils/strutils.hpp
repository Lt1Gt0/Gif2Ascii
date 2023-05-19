#pragma once
#ifndef _STR_UTILS_HPP_
#define _STR_UTILS_HPP_

#include <cstdio>
#include <sstream>
#include <vector>
#include <cstdarg>

using std::string;
using std::vector;

auto inline strFormat(const char* fmt, va_list args) -> string
{
    char buf[8192];
    vsnprintf(buf, 8192, fmt, args);

    return string(buf);
}

auto inline strFormat(const char* fmt, ...) -> string
{
    char buf[8192];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, 8192, fmt, args);
    va_end(args);

    return string(buf);
}

auto inline split(const string& str, char delim) -> vector<string>
{
    vector<string> elems;
    string item;

    std::stringstream stream(str);

    while (getline(stream, item, delim)) {
        elems.push_back(item);
    }

    return elems;
}

#endif // _STR_UTILS_HPP_
