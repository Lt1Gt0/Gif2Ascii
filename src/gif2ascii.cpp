#include "gif.hpp"
#include "display.hpp"
#include "utils/logger.hpp"
#include "utils/error.hpp"

#include <ncurses.h>

/*
    The current version of this converter only works on gif89a not gif87a
    (for the most part). I am not doing to correct reading standard for v87a
    application extensions. Maybe in the future I will add more compatibility
*/

Logger logger;
int main(int argc, char** argv)
{
    // Initialize logger
    logger = Logger("logs/", "info");
    logger.EnableTracing();
        
    if (argc < 2)
        error(Severity::high, "Usage:", "./gif2Ascii <filepath>");

    // Attempt to load GIF
    GIF::File gif(argv[1]);

    Display::InitializeTerminal();
    atexit(Display::ResetTerminal);
    Size winSize = Display::GetDisplaySize();

    logger.Log(TRACE, "(%d, %d)", winSize.width, winSize.height);
    // for (byte i = 0; i < 20; i++) {
        // byte bChannel= i * 10;
        // GIF::Pixel pixel = GIF::Pixel('a', GIF::Color {.red=0, .green=0, .blue=bChannel}, Position{0,0});
        // refresh();
        // pixel.PrintColor();
    // }
    // while(true) {
    //     GIF::Pixel pixel = GIF::Pixel('a', GIF::Color {.red=0, .green=0, .blue=255});
    //     pixel.PrintColor();
    //     refresh();
    //     pixel.PrintColor();
    // }

    // #ifdef DBG
    // gif.DumpInfo("logs/dump.log");
    // #endif

    // GIF::LoopFrames(&gif);

    logger.Close();
    return 0;
}
