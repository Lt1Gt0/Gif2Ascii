#pragma once
#ifndef _PIXEL_HPP_
#define _PIXEL_HPP_

#include <unistd.h>
#include <stdio.h>
#include "utils/types.hpp"

namespace GIF
{
    struct Color {
        byte red;
        byte green;
        byte blue;

        char* ToString() 
        {
            char* colorStr = new char[50];
            sprintf(colorStr, "(%d, %d, %d)", red, green, blue); 
            return colorStr;
        }
    };

    struct Pixel {
        Pixel(char sym, Color c);

        char symbol;
        Color color;

        void PrintColor(FILE* fd = stdout);
        void PrintChar(FILE* fd = stdout);
    };
}

#endif // _PIXEL_HPP_
