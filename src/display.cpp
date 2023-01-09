#include "display.h"
#include "logger.h"
#include "common.h"
#include "lzw.h"

#include <signal.h>
#include <unordered_map>
#include <string.h>
#include <tgmath.h>
#include <thread>
#include <chrono>

namespace GIF
{ 
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

        signal(SIGINT, SigIntHandler);
        std::unordered_map<int, std::string> codeTable = LZW::InitializeCodeTable(gif->mGCTD.ColorCount);
        
        Color* colorTable = nullptr;
        bool useLCT = false;
        if ((gif->mLSD.Packed >> (int)LSDMask::GlobalColorTable) & 0x1)
            colorTable = gif->mGCT;
        else 
            useLCT = true;

        system("clear");
        while (true) {
            int frameIdx = 0;
            
            // Because image data is generic, typecast it to an Image*
            Image* imgData = (Image*)gif->mImageData[frameIdx];

            // if (useLCT) (TODO)
            
            for (std::vector<char> frame : gif->mFrameMap) {
                int col = 0;
                for (char c : frame) {
                    // If for some reason a the character in the map is below zero, break
                    if (c < 0)
                        break;

                    if (c == codeTable.at((int)codeTable.size() - 1)[0]) {
                        LOG_DEBUG << c << " - End of information" << std::endl;
                        break; 
                    }
                    
                    Color color = colorTable[(int)c];
                    if (imgData->mTransparent && c == imgData->mTransparentColorIndex)
                        color = colorTable[imgData->mTransparentColorIndex - 1];
                    
                    fprintf(stdout, "\x1b[38;2;%d;%d;%dm", color.Red, color.Blue, color.Green);
                    fprintf(stdout, "\x1b[48;2;%d;%d;%dm", color.Red, color.Blue, color.Green);
                    fprintf(stdout, "%c", ColorToChar(color));
                    fprintf(stdout, "\x1b[0m");
                    col++;

                    if (col >= gif->mLSD.Width) {
                        col = 0;
                        fprintf(stdout, "\n"); 
                    } 
                } 

                std::this_thread::sleep_for(std::chrono::milliseconds(imgData->mExtensions.GraphicsControl.DelayTime * 10));
                frameIdx++;
                system("clear");
            } 
        }
    }

    char ColorToChar(const Color& color)
    {
        // Brightness in this context is the brighness calculated in grayscale (https://en.wikipedia.org/wiki/Grayscale#Converting_color_to_grayscale)
        float brightness = (0.2126 * color.Red + 0.7152 * color.Green * 0.0722 * color.Blue);
        float chrIdx = brightness / (255.0 / strlen(CHAR_MAP));
        return CHAR_MAP[(int)floor(chrIdx)]; 
    }
}
