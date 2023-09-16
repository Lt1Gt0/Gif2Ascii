#include "gif.hpp"
#include "display.hpp"
#include "utils/logger.hpp"
#include "utils/error.hpp"

#include <stdio.h>

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
        
    if (argc < 2)
        error(Severity::high, "Usage:", "./gif2Ascii <filepath>");

    // Attempt to load GIF
    GIF::File gif(argv[1]);

    #ifdef DBG
    gif.DumpInfo("logs/dump.log");
    #endif

    GIF::LoopFrames(&gif);

    logger.Close();
    return 0;
}
