#include "display.h"
#include "Debug/logger.h"
#include "Debug/debug.h"

GifDisplay::GifDisplay(const GIF* _gif)
{        
    this->mGIF = _gif;
    this->mCharMap = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~i!lI;:,\"^`\'.";
}

GifDisplay::~GifDisplay()
{

}

