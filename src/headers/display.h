#pragma once
#ifndef _GIF_DISPLAY_H
#define _GIF_DISPLAY_H

#include "gif.h"

class GifDisplay 
{
    public:
        GifDisplay(const GIF* _gif);
        ~GifDisplay();

        void LoopFrames();

    private:
        char ColorToChar(const Color& color);
        
    private:
        const GIF* mGIF;
        const char* mCharMap;
};

#endif // _GIF_DISPLAY_H
