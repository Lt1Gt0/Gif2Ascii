#pragma once
#ifndef _GIF_H
#define _GIF_H

#include <vector>
#include <stdio.h>
#include "gifmeta.h"
#include "image.h"

class GIF 
{
    public:
        FILE* file;
        size_t filesize;
        
        GifHeader* header;
        LogicalScreenDescriptor* lsd;
        GlobalColorTableDescriptor* gctd;
        std::vector<Image> imageData;
        std::vector<std::vector<uint8_t>>* colorTable; // If the flag is present then the gct will be filled
        std::vector<std::vector<char>> frameMap;

    public:
        GIF(FILE* fp);
        void ReadFileDataHeaders();
        int LoadHeader();
        void GenerateFrameMap();
        void LoopFrames();
        void PrintHeaderInfo();

    private:
        bool ValidHeader();
};

#endif // _GIF_H