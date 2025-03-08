#include "gif.hpp"
#include "display.hpp"
#include "utils/logger.hpp"
#include "utils/error.hpp"
#include "utils/skutilflagparser.hpp"

#include <ncurses.h>
/*
    The current version of this converter only works on gif89a not gif87a
    (for the most part). I am not doing to correct reading standard for v87a
    application extensions. Maybe in the future I will add more compatibility
*/

char* filepath = nullptr;
void SetFilePath(int inputCount, char** inputVals)
{
    filepath = inputVals[0]; 
}

Logger logger;
int main(int argc, char** argv)
{
    SKUTIL::SK_VEC<SKUTIL::Flag> flags = {
        {
            'f',
            "filepath",
            "Location of gif file to be converted to ascii",
            1,
            SetFilePath
        }
    };

    SKUTIL::FlagParser flagParser = SKUTIL::FlagParser(&flags);
    flagParser.ParseFlags(argc, argv);

    // Initialize logger
    logger = Logger("logs/", "info");
    logger.EnableTracing();
        
    // Attempt to load GIF
    if (filepath == nullptr)
        error(Severity::high, "Usage:", "./gif2Ascii -f <filepath>");

    GIF::File gif(filepath);

    Display::InitializeTerminal();
    atexit(Display::ResetTerminal);
    Size winSize = Display::GetDisplaySize();

    // Test position mapping by positioning the pixel in the midel of the screen
    Display::PixelMap pixMap = Display::PixelMap(winSize.width, winSize.height);

    for (byte i = 0; i < 20; i++) {
        byte hue = 15 * i;
        GIF::Pixel pixel = GIF::Pixel('a', GIF::Color {.red=hue, .green=hue, .blue=hue}, Position{i,i});
        pixMap.InsertPixel(&pixel);
    }

    while(true) {
        Display::DumpPixelMap(&pixMap);
        refresh();
    }

    // #ifdef DBG
    // gif.DumpInfo("logs/dump.log");
    // #endif

    // GIF::LoopFrames(&gif);

    logger.Close();
    return 0;
}
