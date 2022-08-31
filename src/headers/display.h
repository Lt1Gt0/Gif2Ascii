#pragma once
#ifndef _GIF_DISPLAY_H
#define _GIF_DISPLAY_H

#include "gif.h"

namespace GIF
{
    constexpr const char* CHAR_MAP {"$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~i!lI;:,\"^`\'."};
    void LoopFrames(const File* gif);
    char ColorToChar(const Color& color);
}

#endif // _GIF_DISPLAY_H
