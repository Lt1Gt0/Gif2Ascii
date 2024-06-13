#include "display.hpp"
#include "utils/logger.hpp"
#include "lzw.hpp"

#include <signal.h>
#include <unordered_map>
#include <string.h>
#include <tgmath.h>
#include <thread>
#include <chrono>

GifDisplay::GifDisplay(const GIF* _gif)
{
    this->mGIF = _gif;
    this->mCharMap = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~i!lI;:,\"^`\'.";
}

GifDisplay::~GifDisplay() {}

void GifDisplay::LoopFrames()
{
    /* TODO
     * Because of the system("clear") calls, all of the gif meta is
     * destroyed, if I want to see the gif meta I should try to write
     * it into a seperate file before drawing
     */

    signal(SIGINT, this->mGIF->SigIntHandler);
    std::unordered_map<int, std::string> codeTable = LZW::InitializeCodeTable(this->mGIF->mGctd.NumberOfColors);
    
    system("clear");
    while (true) {
        int frameIdx = 0;
        for (std::vector<char> frame : this->mGIF->mFrameMap) {
            int col = 0;
            for (char c : frame) {
                // If for some reason a the character in the map is below zero, break
                if (c < 0)
                    break;

                if (c == codeTable.at((int)codeTable.size() - 1)[0]) {
                    logger.Log(DEBUG, " - End of information");
                    break; 
                }
                
                Color color = this->mGIF->mColorTable[(int)c];
                if (this->mGIF->mImageData[frameIdx].mTransparent 
                && c == this->mGIF->mImageData[frameIdx].mTransparentColorIndex) {
                    // Add transparent color  
                    color = this->mGIF->mColorTable[this->mGIF->mImageData[frameIdx].mTransparentColorIndex - 1];
                } 
                
                fprintf(stdout, "\x1b[38;2;%d;%d;%dm", color.Red, color.Blue, color.Green);
                fprintf(stdout, "\x1b[48;2;%d;%d;%dm", color.Red, color.Blue, color.Green);
                fprintf(stdout, "%c", ColorToChar(color));
                fprintf(stdout, "\x1b[0m");
                col++;

                if (col >= this->mGIF->mLsd.Width) {
                    col = 0;
                    fprintf(stdout, "\n"); 
                } 
            } 

            std::this_thread::sleep_for(std::chrono::milliseconds(this->mGIF->mImageData[frameIdx].mExtensions.GraphicsControl.DelayTime * 10));
            frameIdx++;
            system("clear");
        } 
    }
}

char GifDisplay::ColorToChar(Color& color)
{
    // Brightness in this context is the brighness calculated in grayscale (https://en.wikipedia.org/wiki/Grayscale#Converting_color_to_grayscale)
    float brightness = (0.2126 * color.Red + 0.7152 * color.Green * 0.0722 * color.Blue);
    float chrIdx = brightness / (255.0 / strlen(this->mCharMap));
    return this->mCharMap[(int)floor(chrIdx)]; 
}


void Color::Print()
{
    // Background
    fprintf(stdout, "\x1b[38;2;%d;%d;%dm", Red, Green, Blue);

    // Foreground
    fprintf(stdout, "\x1b[48;2;%d;%d;%dm", Red, Green, Blue);
    fprintf(stdout, "%c", GifDisplay::ColorToChar(this));
    fprintf(stdout, "\x1b[0m");

}
