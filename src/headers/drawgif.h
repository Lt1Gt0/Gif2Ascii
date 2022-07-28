#pragma once
#ifndef _DRAW_GIF_H
#define _DRAW_GIF_H

#include "gif.h"
#include "image.h"
#include "lzw.h"
#include "image.h"
#include <vector>
#include <stdint.h>

namespace Draw
{
    /**
     * Initialize terminal window for drawing
     * this will call different ncurses methods
     * to check if the terminal has features
     *
     * @param colorTableSize - If you know that your terminal
     * can support colors from ncurses, you can pas the size of 
     * the color table your gif generated
     */
    void Initialize(const GIF& gif);
    void InitializeColorMap(const Color* colorTable, int colorTableSize);
    
    void LoopFrames(const GIF&  gif);
    char ColorToChar(const char* charMap, const Color& color);

    void End();
}

#endif // _DRAW_GIF_H
