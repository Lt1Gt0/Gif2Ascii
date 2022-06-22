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
        FILE* file;
        Header* header;
        LogicalScreenDescriptor* lsd;
        GlobalColorTableDescriptor* gctd;
        void* gct;
        GraphicsControlExtension* gce;
        ImageDescriptor* imageDescriptor;
        // LocalColorTable* lct;
        // ImageData* imageData;
        // PlainTextExtension* pte;
        // CommentExtension* ce; 
        // Trailer* trailer;

        GIF();
        void ReadFileDataHeaders(const char* filepath);
        bool ValidHeader();
        void PrintHeaderInfo();

    private:
        // bool ValidHeader(uint8_t mag[6]);
};

#endif // _GIF_H