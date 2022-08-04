#include "gif.h"
//#include "drawgif.h"
#include "display.h"
#include "Debug/debug.h"
#include "Debug/logger.h"

/*
    The current version of this converter only works on gif89a not gif87a
    at least I belive so. I am not doing to correct reading standard for v87a
    fot application extensions and possibly the other 3 extension types. Maybe
    in the future I will add more compatibility
*/

Logger logger;
int main(int argc, char** argv)
{
    // Initialize logger
    LOG_INIT("logs/", "info")
        
    if (argc < 2)
        Debug::error(Severity::high, "Usage:", "./bin/gif2Ascii <filepath>");

    // Attempt to load GIF
    const char* filepath = argv[1];
    FILE* fp = fopen(filepath, "rb");

    if (fp == NULL)
        Debug::error(Severity::high, "Error opening file:", filepath);
    else
        LOG_SUCCESS << "Opened [" << filepath << "]" << std::endl;

    GIF gif = GIF(fp);
    LOG_INFO << "Reading GIF Information" << std::endl;
    gif.Read();

    // Setup drawing procdure and display frame data
    GifDisplay display = GifDisplay(&gif);
    display.LoopFrames();

    logger.Close();
    return 0;
}
