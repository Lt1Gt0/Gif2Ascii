#include "common.h"
#include "logger.h"
#include "gif.h"

Logger logger;
int main(int argc, char* argv[])
{
    LOG_INIT("logs/", "info")

    if (argc < 2)
        Debug::error(Severity::high, "Usage:", "./gif2Ascii <filepath>");

    // Initialize GIF
    GIF::File file = GIF::File(argv[1]);

    if (GIF::Initialize(&file) == GIF::Status::failure)
        Debug::error(Severity::high, "Unable to initialize GIF");

    file.Read();

    file.DumpInfo("logs/dump");
    
    // Read GIF Data
    //
    // Display GIF Image

    return 0;
}

//#include "gif.h"
//#include "display.h"
//#include "Debug/debug.h"
//#include "Debug/logger.h"

//[>
    //The current version of this converter only works on gif89a not gif87a
    //(for the most part). I am not doing to correct reading standard for v87a
    //application extensions. Maybe in the future I will add more compatibility
//*/

//Logger logger;
//int main(int argc, char** argv)
//{
    //// Initialize logger
    //LOG_INIT("logs/", "info")
        
    //if (argc < 2)
        //Debug::error(Severity::high, "Usage:", "./bin/gif2Ascii <filepath>");

    //// Attempt to load GIF
    //GIF gif = GIF(argv[1]);
    //gif.Read();

    //// Setup drawing procdure and display frame data
    //GifDisplay display = GifDisplay(&gif);
    //display.LoopFrames();

    //logger.Close();
    //return 0;
//}
