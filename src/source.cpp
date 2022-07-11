#include "gif.h"
#include "errorhandler.h"
#include "Debug/logger.h"

/*
    The current version of this converter only works on gif89a not gif87a
    at least I belive so. I am not doing to correct reading standard for v87a
    fot application extensions and possibly the other 3 extension types. Maybe
    in the future I will add more compatibility
*/

int main(int argc, char** argv)
{
    if (argc < 2)
        ErrorHandler::err_n_die("Usage: %s <filepath>", argv[0]);

    LOG_INIT("logs/", "info")
    LOG_INFO << "Bruh\n";
    
    const char* filepath = argv[1];
    
    FILE* fp = fopen(filepath, "rb");
    if (fp == NULL)
        ErrorHandler::err_n_die("Error opening file [%s]", filepath);
    else
        LOG_INFO << "Successfully opened [" << filepath << "]\n";

    GIF* gif = new GIF(fp);
    LOG_INFO << "Reading information from file\n";
    
    gif->ReadFileHeaders();
    gif->GenerateFrameMap();
    gif->LoopFrames();

    logger.Close();
    return 0;
}