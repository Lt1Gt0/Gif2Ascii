#pragma once
#ifndef _GIF_H
#define _GIF_H

#include <stdint.h>
#include <stdio.h>
#include "gifheaders.h"
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
        void* gct;
        // GraphicsControlExtension* gce;
        ImageDescriptor* imageDescriptor;
        // LocalColorTable* lct;
        // ImageData* imageData;
        // PlainTextExtension* pte;
        // CommentExtension* ce; 
        // Trailer* trailer;

        void ReadFileDataHeaders();
        int LoadHeader();
        void DataLoop();
        void PrintHeaderInfo();
        void PrintImageDescriptor(ImageDescriptor* descriptor);
        void PrintImageData(ImageData* data);

    private:
        bool ValidHeader();
        void LoadExtension(ExtensionHeader* header);
        void LoadImageData();
        uint8_t* ReadImgDataSubBlock(ImageData* ImgData);
        void CheckExtensions();
};

#endif // _GIF_H