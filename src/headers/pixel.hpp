#pragma once
#ifndef _PIXEL_HPP_
#define _PIXEL_HPP_

#include <unistd.h>

#include "gifmeta.hpp"

namespace GIF
{
    struct Pixel {
        Pixel(char sym, Color c);

        char mSymbol;
        Color mColor;

        void PrintColor(FILE* fd = stdout);
        void PrintChar(FILE* fd = stdout);
    };
}

#endif // _PIXEL_HPP_
