#include "gif.hpp"
#include "display.hpp"
#include "utils/error.hpp"
#include "utils/logger.hpp"

/*
    The current version of this converter only works on gif89a not gif87a
    (for the most part). I am not doing to correct reading standard for v87a
    application extensions. Maybe in the future I will add more compatibility
*/

Logger logger;
int main(int argc, char** argv)
{
    // Initialize logger
    logger = new Logger("logs/", "info");
    logger.EnableTracing();
        
    if (argc < 2)
        error(Severity::high, "Usage:", "./bin/gif2Ascii <filepath>");

    // Attempt to load GIF
    GIF gif = GIF(argv[1]);
    gif.Read();

    // Setup drawing procdure and display frame data
    // GifDisplay display = GifDisplay(&gif);
    // display.LoopFrames();

    logger.Close();
    return 0;
}
