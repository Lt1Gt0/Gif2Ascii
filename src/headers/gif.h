#pragma once
#ifndef _GIF_H
#define _GIF_H

#include <stdint.h>
#include <stdio.h>
#include <vector>
#include "gifheaders.h"
#include "imagedata.h"
#include "lzw.h"
// https://docs.fileformat.com/image/gif/

using namespace GIF_Headers;
class GIF 
{
    public:
        GIF(FILE* fp);
        
        FILE* file;
        size_t filesize;
        
        Header* header;
        LogicalScreenDescriptor* lsd;
        GlobalColorTableDescriptor* gctd;
        std::vector<ImageData::Image> imageData;
        std::vector<std::vector<uint8_t>> colorTable; // If the flag is present then the gct will be filled

        void ReadFileDataHeaders();
        int LoadHeader();
        void DataLoop();
        void PrintHeaderInfo();

    private:
        bool ValidHeader();
        void LoadExtension(ExtensionHeader* header);
        void LoadImageData();
        void CheckExtensions();
};

#endif // _GIF_H