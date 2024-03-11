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
        Pixel(char sym, Color c, Position pos);

        char symbol;
        Color color;
        Position position;

        void PrintColor(FILE* fd = stdout);
        void PrintChar(FILE* fd = stdout);

        void SetPos(Position pos);
    };
}

#endif // _PIXEL_HPP_
