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
        
            // These two members should eventually be treated as private
            size_t mRows;
            size_t mCols;

        private:
            GIF::Pixel** mBase;
            Size mMaxSize;
    };

    void InitializeTerminal();
    void ResetTerminal();
    Size GetDisplaySize();
    
    // For whatever reason if you would like to specify a new screen size for the gif
    // to be drawn in, use this method
    // 
    // It will check to see that the new screen size will not be larger than what the current
    // terminal size is so it won't draw out of bounds
    void SetDisplaySize(Size newSize);

    void DumpPixelMap(PixelMap* pixMap);
}

namespace GIF
{
    constexpr const char* CHAR_MAP {"$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~i!lI;:,\"^`\'."};

    void LoopFrames(const File* gif);
    char ColorToChar(const Color& color);
}

#endif // _GIF_DISPLAY_HPP_
