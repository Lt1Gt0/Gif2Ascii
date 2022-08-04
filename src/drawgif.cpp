#include "drawgif.h"
#include "Debug/logger.h"
#include "gifmeta.h"

#include <ncurses.h>
#include <unordered_map>
#include <string.h>
#include <tgmath.h>
#include <thread>
#include <chrono>

namespace Draw
{
    void Initialize(const GIF& gif)
    {         
        LOG_INFO << "Initializing gif drawing" << std::endl;
        
        // Initialize ncurses
        initscr();

        // Check if terminal supports colors 
        if (has_colors() == TRUE) {
            // If the GIF has a GCT then load the color map to the gct
            if ((gif.mLsd->Packed >> (uint8_t)LSDMask::GlobalColorTable) & 0x01) {
                start_color();
                InitializeColorMap(gif.mColorTable, gif.mGctd->NumberOfColors); 
            }
        }
        
        clear();
    }

    void InitializeColorMap(const Color* colorTable, int colorTableSize) 
    {  
        reset_color_pairs();
        for (int pair = 0, color = 0; pair < colorTableSize; pair++, color++) {
            init_extended_color(color, colorTable[color].Red, colorTable[color].Green, colorTable[color].Blue); 
            init_extended_pair(pair, color, COLOR_BLACK);
        } 
    }

    void LoopFrames(const GIF& gif)
    {
        // This is where things get a bit more confusing and will most likely be completely
        // reworked to make more sense
 
        std::unordered_map<int, std::string> codeTable = LZW::InitializeCodeTable(gif.mGctd->NumberOfColors);
 
        // standard char map for grayscale ascii color represnetation
        const char* charMap = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~i!lI;:,\"^`\'.";
 
        clear();
        while (true) {
            int frameIdx = 0;
 
            // Go through each frame in the frame map
            for (std::vector<char> frame : gif.mFrameMap) {
                
                // Initialize variables for Image Display
                int col = 0;
 
                // Go through each code in the frame
                for (char c : frame) {
                    if (col >= gif.mLsd->Width) {
                        col = 0;
                        printw("\n");
                    }
 
                    if (c == codeTable.at((int)codeTable.size() - 1)[0]) {
                        LOG_DEBUG << c << " - End of information" << std::endl;
                        break;
                    }
 
                    // If for some reason a character below 0 then leave the loop
                    if ((int)c < 0) 
                        break;
 
                    // Check to see if the transparency flag is set in the image if so
                    // check if the current code is the transparency color index
                    if (gif.mImageData[frameIdx].mTransparent && (int)c == gif.mImageData[frameIdx].mTransparentColorIndex) {
                        addstr(TRANSPRENT_CHAR); 
                    } else {
                        Color color = gif.mColorTable[(int)c];
                        attron(COLOR_PAIR(int(c)));
                        addch(ColorToChar(charMap, color));
                        attron(COLOR_PAIR(int(c)));
                    }
 
                    col++;
                }
 
                std::this_thread::sleep_for(std::chrono::milliseconds(gif.mImageData[frameIdx].mExtensions->GraphicsControl->DelayTime * 10));
 
                frameIdx++;
                refresh();
                clear();
            }
        }
    }

    char ColorToChar(const char* charMap, const Color& color)
    {
        // Brightness in this context is the brighness calculated in grayscale (https://en.wikipedia.org/wiki/Grayscale#Converting_color_to_grayscale)
        float brightness = (0.2126 * color.Red + 0.7152 * color.Green * 0.0722 * color.Blue);
        float chrIdx = brightness / (255.0 / strlen(charMap));
        char c = charMap[(int)floor(chrIdx)]; 
        return c;
    }

    void End()
    {
        endwin(); 
    }
}
