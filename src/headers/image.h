#pragma once
#ifndef _GIF_IMAGE_DATA_H
#define _GIF_IMAGE_DATA_H

#include "imagemeta.h"
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>

class Image 
{            
    public:
        FILE* file;
        ImageDescriptor* descriptor;
        ImageDataHeader* header;
        ImageExtensions* extensions;
        std::vector<std::vector<uint8_t>>* colorTable;
        std::vector<uint8_t> data;
        
    public:
        Image(FILE* fp, std::vector<std::vector<uint8_t>>* colortable);
        void PrintData();
        void ReadDataSubBlocks(FILE* file);
        void LoadExtension(ExtensionHeader* header);

        // Return the charstream given after decompression
        std::string LoadImageData();
        void CheckExtensions();

        std::vector<char> UpdateFrame(std::string* rasterData, std::vector<char>* origPixMap, int gifWidth, int gifHeight);

    private:
        void PrintDescriptor();
        void PrintSubBlockData(std::vector<uint8_t> block);

        // Different Drawing behaviors based off Disposal Methods
        void DrawOverImage(std::string* rasterData, std::vector<char>* pixelMap, int gifWidth, int gifHeight);
        void RestoreCanvasToBG(std::string* rasterData, std::vector<char>* pixelMap);
        void RestoreToPrevState(std::string* rasterData, std::vector<char>* pixelMap);
};

#endif // _GIF_IMAGE_DATA_H