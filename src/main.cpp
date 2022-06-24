#include "gif.h"
#include "errorhandler.h"
#include "Debug/debug.h"

/*
    The current version of this converter only works on gif89a not gif87a
    at least I belive so. I am not doing to correct reading standard for v87a
    fot application extensions and possibly the other 3 extension types. Maybe
    in the future I will add more compatibility
*/

int main(int argc, char** argv)
{
    if (argc < 2) {
        ErrorHandler::err_n_die("Usage: %s <filepath>", argv[0]);
    }
    const char* filepath = argv[1];
    
    FILE* fp = fopen(filepath, "rb");
    if (fp == NULL) {
        ErrorHandler::err_n_die("Error opening file [%s]", filepath);
    } else {
        Debug::Print("Successfully opened [%s]", filepath);
    }

    GIF* gif = new GIF(fp);
    Debug::Print("Reading File Information Data...");
    
    gif->ReadFileDataHeaders();
    // gif.PrintHeaderInfo(); // Debug
    gif->GenerateFrameMap();
    // gif->LoopFrames();

    return 0;
}