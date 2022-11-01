#include "gif.h"
#include "display.h"
#include "logger.h"

/*
    The current version of this converter only works on gif89a not gif87a
    (for the most part). I am not doing to correct reading standard for v87a
    application extensions. Maybe in the future I will add more compatibility
*/

Logger logger;
int main(int argc, char** argv)
{
    // Initialize logger
    LOG_INIT("logs/", "info")
        
    if (argc < 2)
        error(Severity::high, "Usage:", "./bin/gif2Ascii <filepath>");

    // Attempt to load GIF
    GIF::File gif = GIF::File(argv[1]);
    gif.Read();
    gif.DumpInfo("logs/dump");

    GIF::LoopFrames(&gif);

    logger.Close();
    return 0;
}
