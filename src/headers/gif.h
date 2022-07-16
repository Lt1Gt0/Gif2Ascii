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
        FILE* mFile;
        
        GifHeader* mHeader;
        LogicalScreenDescriptor* mLsd;
        GlobalColorTableDescriptor* mGctd;
        std::vector<Image> mImageData;
        Color* mColorTable; // If the flag is present then the gct will be filled
        std::vector<std::vector<char>> mFrameMap;

    public:
        GIF(FILE* _fp);
        void ReadFileDataHeaders();
        void GenerateFrameMap();
        void LoopFrames();

    private:
        size_t mFilesize;
        
        void LoadHeader();
        bool ValidHeader();
        
        // Debug Prints
        void PrintHeaderInfo();
        void PrintColorTable();
};

#endif // _GIF_H
