#include "common.h"
#include "display.h"
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
    
    // Display GIF Image
    //GIF::LoopFrames(&file);

    logger.Close();
    return 0;
}
