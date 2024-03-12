#pragma once
#ifndef _GIF_DISPLAY_HPP_
#define _GIF_DISPLAY_HPP_

#include "gif.hpp"

namespace Display
{
    struct PixelMap {
        public: 
            PixelMap(size_t rows, size_t cols);
            ~PixelMap();

            int InsertPixel(GIF::Pixel* pix); 

            GIF::Pixel at(size_t row, size_t col);
        
        public:
            size_t mRows;
            size_t mCols;
            GIF::Pixel** mBase;
    };

    void InitializeTerminal();
    void ResetTerminal();
    Size GetDisplaySize();

    void DumpPixelMap(PixelMap* pixMap);
}

namespace GIF
{
    constexpr const char* CHAR_MAP {"$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~i!lI;:,\"^`\'."};

    void LoopFrames(const File* gif);
    char ColorToChar(const Color& color);
}

#endif // _GIF_DISPLAY_HPP_
