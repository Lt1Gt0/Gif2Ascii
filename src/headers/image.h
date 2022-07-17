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
        ImageDescriptor* mDescriptor;
        ImageDataHeader* mHeader;
        ImageExtensions* mExtensions;
        Color* mColorTable;
        std::vector<uint8_t> mData;
        
    public:
        Image(FILE* _fp, Color* _colortable, uint8_t _colorTableSize);
        
        std::string LoadImageData();
        void ReadDataSubBlocks();
        void CheckExtensions();

        // Return the charstream given after decompression
        void UpdatePixelMap(std::vector<char>* pixMap, std::string* rasterData, LogicalScreenDescriptor* lsd);

    private:
        FILE* mFile;
        uint8_t mColorTableSize;
    
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
