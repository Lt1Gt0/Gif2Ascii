#include "display.h"
#include "lzw.h"

#include <signal.h>
#include <unordered_map>
#include <string>
#include <tgmath.h>
#include <string.h>

namespace GIF
{
    void LoopFrames(const File* gif)
    {
        signal(SIGINT, sigIntHandler);
        std::unordered_map<int, std::string> codeTable = LZW::InitializeCodeTable(gif->mGCTD.colorCount);
        system("clear");
        
        while (true) {
            int frameIdx = 0;
            for (char** frame : gif->mFrameMap) {
                int col = 0;
                
                // TODO
                /* for (char c : frame) */ {
                    //// If for some reason a the character in the map is below zero, break
                    //if (c < 0)
                        //break;

                    //if (c == codeTable.at((int)codeTable.size() - 1)[0]) {
                        //LOG_DEBUG << c << " - End of information" << std::endl;
                        //break; 
                    //}
                    
                    //Color color = this->mGIF->mColorTable[(int)c];
                    //if (this->mGIF->mImageData[frameIdx].mTransparent 
                    //&& c == this->mGIF->mImageData[frameIdx].mTransparentColorIndex) {
                        //// Add transparent color  
                        //color = this->mGIF->mColorTable[this->mGIF->mImageData[frameIdx].mTransparentColorIndex - 1];
                    //} 
                    
                    //fprintf(stdout, "\x1b[38;2;%d;%d;%dm", color.Red, color.Blue, color.Green);
                    //fprintf(stdout, "\x1b[48;2;%d;%d;%dm", color.Red, color.Blue, color.Green);
                    //fprintf(stdout, "%c", ColorToChar(color));
                    //fprintf(stdout, "\x1b[0m");
                    //col++;

                    //if (col >= this->mGIF->mLsd.Width) {
                        //col = 0;
                        //fprintf(stdout, "\n"); 
                    //} 
                //} 

                //std::this_thread::sleep_for(std::chrono::milliseconds(this->mGIF->mImageData[frameIdx].mExtensions.GraphicsControl.DelayTime * 10));
                //frameIdx++;
                //system("clear");
                } 
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
