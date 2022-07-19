#include "gif.h"
#include "errorhandler.h"
#include "Debug/debug.h"
#include "Debug/logger.h"

/*
    The current version of this converter only works on gif89a not gif87a
    at least I belive so. I am not doing to correct reading standard for v87a
    fot application extensions and possibly the other 3 extension types. Maybe
    in the future I will add more compatibility
*/

int main(int argc, char** argv)
{
    // Initialize logger
    LOG_INIT("logs/", "info")
        
    if (argc < 2)
        ErrorHandler::err_n_die("Usage: %s <filepath>", argv[0]);

    // Attempt to load GIF
    const char* filepath = argv[1];
    FILE* fp = fopen(filepath, "rb");

    if (fp == NULL)
        ErrorHandler::err_n_die("Error opening file [%s]", filepath);
    else
        LOG_SUCCESS << "Opened [" << filepath << "]" << std::endl;

    GIF* gif = new GIF(fp);
    LOG_INFO << "Reading GIF Information" << std::endl;
    
    gif->Read();
    gif->LoopFrames();

    logger.Close();
    return 0;
}
