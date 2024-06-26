#pragma once
#ifndef _GIF_IMAGE_DATA_HPP
#define _GIF_IMAGE_DATA_HPP

#include "imagemeta.hpp"
#include "gifmeta.hpp"
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>

class Image 
{            
    public:
        ImageDescriptor mDescriptor;
        ImageDataHeader mHeader;
        ImageExtensions mExtensions;
        Color* mColorTable;
        std::vector<uint8_t> mData;

        bool mTransparent;
        uint8_t mTransparentColorIndex;
        
    public:
        Image(FILE* _fp, Color* _colortable, uint8_t _colorTableSize);
        
        std::string LoadImageData();
        void ReadDataSubBlocks();
        void CheckExtensions();

        // Return the charstream given after decompression
        void UpdatePixelMap(std::vector<char>* pixMap, std::vector<char>* prevPixMap, std::string* rasterData, LogicalScreenDescriptor* lsd);

    private:
        FILE* mFile;
        uint8_t mColorTableSize;
    
    private:
        // Different Drawing behaviors based off Disposal Methods
        void DrawOverImage(std::string* rasterData, std::vector<char>* pixelMap, LogicalScreenDescriptor* lsd);
        void RestoreCanvasToBG(std::vector<char>* pixelMap, LogicalScreenDescriptor* lsd);
        void RestoreToPrevState(std::vector<char>* pixMap, std::vector<char>* prevPixMap);
        
        void LoadExtension(const ExtensionHeader& headerCheck);
        
        // Debugging prints
        void PrintDescriptor();
        void PrintData();
        void PrintSubBlockData(std::vector<uint8_t>* block);
};

#endif // _GIF_IMAGE_DATA_HPP
