#include "display.hpp"
#include "utils/logger.hpp"
#include "utils/types.hpp"
#include "lzw.hpp"
#include "pixel.hpp"

#include <signal.h>
#include <unordered_map>
#include <string.h>
#include <tgmath.h>
#include <thread>
#include <chrono>
#include <termios.h>
#include <unistd.h>

namespace GIF
{ 
    termios gOrigTerm;
    termios gCurrentTerm;

    void InitializeTerminal()
    {
        // Get initial terminal attributes
        tcgetattr(STDOUT_FILENO, &gOrigTerm);

        // Set new termial attributes
        // gCurrentTerm = gOrigTerm;
        // gCurrentTerm.c_iflag 
    }

    void ResetTerminal()
    {
        tcsetattr(STDOUT_FILENO, 0, &gOrigTerm);
    }


    void SigIntHandler(int sig)
    {
        system("clear");        
        exit(0);
    }

    void LoopFrames(const File* gif)
    {
        /* TODO
         * Because of the system("clear") calls, all of the gif meta is
         * destroyed, if I want to see the gif meta I should try to write
         * it into a seperate file before drawing
         */

        logger.Log(DEBUG, "Looping Frames");

        signal(SIGINT, SigIntHandler);
        std::unordered_map<int, std::string> codeTable = LZW::InitializeCodeTable(gif->mDS.gctd.colorCount);
        
        Color* colorTable = nullptr;
        bool useLCT = false;

        if ((gif->mDS.lsd.packed >> (int)GIF::LogicalScreen::Mask::GlobalColorTable) & 0x1)
            colorTable = gif->mGCT;
        else 
            useLCT = true;

        system("clear");
        while (true) {
            // int frameIdx = 0;
            // 
            // // Because image data is generic, typecast it to an Image*
            // Image* imgData = reinterpret_cast<Image*>(gif->mImageData[frameIdx]);
            //
            // // if (useLCT) (TODO)
            // 
            // FILE* output = stdout;
            // for (std::vector<char> frame : gif->mFrameMap) {
            //     int col = 0;
            //     for (char c : frame) {
            //         // If for some reason a the character in the map is below zero, break
            //         if (c < 0)
            //             break;
            //
            //         if (c == codeTable.at((int)codeTable.size() - 1)[0]) {
            //             logger.Log(DEBUG, "%c - End of information", c);
            //             break; 
            //         }
            //         
            //         logger.Log(DEBUG, "C: %c", c);
            //         Color color = colorTable[(int)c];
            //         if (imgData->mTransparent && c == imgData->mTransparentColorIndex)
            //             color = colorTable[imgData->mTransparentColorIndex - 1];
            //
            //         GIF::Pixel p = Pixel(ColorToChar(color), color);
            //         p.PrintColor(output);
            //
            //         col++;
            //
            //         if (col >= gif->mDS.lsd.width) {
            //             col = 0;
            //             fprintf(output, "\n"); 
            //         } 
            //     } 
            //
            //     std::this_thread::sleep_for(std::chrono::milliseconds(imgData->mData->graphic.gce.delayTime * 100));
            //     frameIdx++;
            //     system("clear");
            // } 
            break;
        }
    }


    char ColorToChar(const GIF::Color& color)
    {
        // Brightness in this context is the brighness calculated in grayscale (https://en.wikipedia.org/wiki/Grayscale#Converting_color_to_grayscale)
        float brightness = (0.2126 * color.red + 0.7152 * color.green * 0.0722 * color.blue);
        float chrIdx = brightness / (255.0 / strlen(GIF::CHAR_MAP));
        return GIF::CHAR_MAP[(int)floor(chrIdx)]; 
    }
}
