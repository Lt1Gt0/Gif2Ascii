#include "display.hpp"
#include "utils/logger.hpp"
#include "utils/types.hpp"
#include "lzw.hpp"
#include "pixel.hpp"

#include <signal.h>
#include <unordered_map>
#include <assert.h>
#include <string.h>
#include <tgmath.h>
#include <thread>
#include <chrono>
#include <termios.h>
#include <unistd.h>
#include <ncurses.h>

namespace Display 
{
    Size* gDisplaySize;

    void InitializeTerminal()
    {
        gDisplaySize = nullptr;

        // Initialize ncurses
        initscr();
        cbreak();
        noecho();

        // Get the display size information
        int row;
        int col;
        gDisplaySize = new Size;
        getmaxyx(stdscr, row, col);
        gDisplaySize->width = col;
        gDisplaySize->height = row; 
    }

    void ResetTerminal()
    {
        endwin();
    }

    Size GetDisplaySize()
    {
        assert(gDisplaySize != nullptr);
        return *gDisplaySize;
    }

    void DumpPixelMap(PixelMap* pixMap)
    {
        for (size_t row = 0; row <= pixMap->mRows; row++) {
            for (size_t col = 0; col <= pixMap->mCols; col++) {
                GIF::Pixel pixel = pixMap->at(row, col);
                if (pixel.CheckPosInBounds(*gDisplaySize)) {
                    mvaddch(col, row, pixel.symbol);
                }
            }
        }
    }

    GIF::Pixel PixelMap::at(size_t row, size_t col)
    {
        return mBase[col][row];
    }

    int PixelMap::InsertPixel(GIF::Pixel* pix)
    {
        mBase[pix->position.Y][pix->position.X] = *pix;
        return 0;
    }

    PixelMap::PixelMap(size_t rows, size_t cols)
    {
        this->mRows = rows;
        this->mCols = cols;

        this->mBase = (GIF::Pixel**)malloc(mRows * sizeof(GIF::Pixel*));
        if (this->mBase != nullptr) {
            for (size_t row = 0; row <= mRows; row++) {
                this->mBase[row] = (GIF::Pixel*)malloc(mCols * sizeof(GIF::Pixel*));
            }
        }
    }

    PixelMap::~PixelMap() { }
}

namespace GIF
{ 
    void LoopFrames(const File* gif)
    {
        /* TODO
         * Because of the system("clear") calls, all of the gif meta is
         * destroyed, if I want to see the gif meta I should try to write
         * it into a seperate file before drawing
         */

        logger.Log(TRACE, "Looping Frames");
        std::unordered_map<int, std::string> codeTable = LZW::InitializeCodeTable(gif->mDS.gctd.colorCount);
        
        Color* colorTable = nullptr;
        bool useLCT = false;

        if ((gif->mDS.lsd.packed >> (int)GIF::LogicalScreen::Mask::GlobalColorTable) & 0x1)
            colorTable = gif->mGCT;
        else 
            useLCT = true;

        refresh();
        while (true) {
            int frameIdx = 0;

            // Because image data is generic, typecast it to an Image*
            Data::Data imgData = gif->mDS.data[frameIdx];

            // if (useLCT) (TODO)

            FILE* output = stdout;
            for (std::vector<char> frame : gif->mFrameMap) {
                int col = 0;
                for (char c : frame) {
                    // If for some reason a the character in the map is below zero, break
                    if (c < 0)
                        break;

                    if (c == codeTable.at((int)codeTable.size() - 1)[0]) {
                        logger.Log(DEBUG, "%c - End of information", c);
                        break; 
                    }
                    
                    Color color = colorTable[(int)c];
                    bool transparent = (imgData.graphic.gce.packed >> (int)Data::Graphic::GCEMask::TransparentColor) & 0x1;

                    if (transparent && c == imgData.graphic.gce.transparentColorIndex)
                        color = colorTable[imgData.graphic.gce.transparentColorIndex - 1];

                    // FIXME
                    GIF::Pixel p = Pixel(ColorToChar(color), color, Position{0,0});
                    p.PrintColor(output);

                    col++;

                    if (col >= gif->mDS.lsd.width) {
                        col = 0;
                        fprintf(output, "\n"); 
                    } 
                } 

                std::this_thread::sleep_for(std::chrono::milliseconds(imgData.graphic.gce.delayTime * 100));
                frameIdx++;
                refresh();
            } 
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
