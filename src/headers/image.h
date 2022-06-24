#pragma once
#ifndef _GIF_IMAGE_DATA_H
#define _GIF_IMAGE_DATA_H

#include "imagemeta.h"
#include "gifmeta.h"
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
        
        std::string LoadImageData();
        void ReadDataSubBlocks(FILE* file);
        void CheckExtensions();

        // Return the charstream given after decompression
        void UpdatePixelMap(std::vector<char>* pixMap, std::string* rasterData, LogicalScreenDescriptor* lsd);

    private:
        // Different Drawing behaviors based off Disposal Methods
        void DrawOverImage(std::string* rasterData, std::vector<char>* pixelMap, LogicalScreenDescriptor* lsd);
        void RestoreCanvasToBG(std::string* rasterData, std::vector<char>* pixelMap);
        void RestoreToPrevState(std::string* rasterData, std::vector<char>* pixelMap);
        
        void LoadExtension(ExtensionHeader* header);
        
        // Debugging prints
        void PrintDescriptor();
        void PrintData();
        void PrintSubBlockData(std::vector<uint8_t>* block);

};

#endif // _GIF_IMAGE_DATA_H