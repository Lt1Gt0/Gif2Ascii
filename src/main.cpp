#include "gif.h"

/*
    The current version of this converter only works on gif89a not gif87a
    at least I belive so. I am not doing to correct reading standard for v87a
    fot application extensions and possibly the other 3 extension types. Maybe
    in the future I will add more compatibility
*/

int main()
{
    const char* filepath = "imgs/red_small.gif";
    
    FILE* fp = fopen(filepath, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file [%s]\n", filepath);
        return 1;
    } else {
        fprintf(stdout, "Successfully opened [%s]\n", filepath);
    }

    GIF* gif = new GIF(fp);
    fprintf(stdout, "Reading File Information Data...\n");
    
    gif->ReadFileDataHeaders();
    // gif.PrintHeaderInfo(); // Debug
    gif->GenerateFrameMap();
    gif->LoopFrames();

    return 0;
}