#pragma once
#ifndef _GIF_IMAGE_DATA_H
#define _GIF_IMAGE_DATA_H

#define PACKED __attribute__((packed))

#include "gif.h"
#include "common.h"
#include <vector>

namespace Image
{
    namespace Meta
    {
    }

    struct Image : private GIF::File {
        public:
            GIF::Color*             mColorTable;
            GIF::ImageDescriptor    mDescriptor;
            GIF::ImageDataHeader    mDataHeader;
            GIF::ImageExtensions    mExtensions;
            std::vector<byte>       mData;
            bool                    mTransparent;
            byte                    mTransparentColorIndex;

            // TODO
            Image(const GIF::File& gif) : mGIF(gif) {}

            std::string LoadData();
            void ReadDataSubBlocks();
            void CheckExtensions();

            //void UpdatePixelMap()
            
            // TODO
        private:
            const GIF::File& mGIF;
            void DrawOverImage();
            void RestoreCanvasToBG();
            void RestoreToPrevState();

            void LoadExtension(const Meta::ExtensionHeader& headerCheck);
            
            // Debug Prints
            //void PrintDescriptor();
            //void printData();
            //void PrintSubBlockData();
    };
}

//#include "imagemeta.h"
//#include "gifmeta.h"
//#include <stdio.h>
//#include <stdint.h>
//#include <string>
//#include <vector>

//class Image 
//{            
    //public:
        //Color* mColorTable;
        //ImageDescriptor mDescriptor;
        //ImageDataHeader mHeader;
        //ImageExtensions mExtensions;
        //std::vector<uint8_t> mData;

        //bool mTransparent;
        //uint8_t mTransparentColorIndex;
        
    //public:
        //Image(FILE* _fp, Color* _colortable, uint8_t _colorTableSize);
        
        //std::string LoadImageData();
        //void ReadDataSubBlocks();
        //void CheckExtensions();

        //// Return the charstream given after decompression
        //void UpdatePixelMap(std::vector<char>* pixMap, std::vector<char>* prevPixMap, std::string* rasterData, LogicalScreenDescriptor* lsd);

    //private:
        //FILE* mFile;
        //uint8_t mColorTableSize;
    
    //private:
        //// Different Drawing behaviors based off Disposal Methods
        //void DrawOverImage(std::string* rasterData, std::vector<char>* pixelMap, LogicalScreenDescriptor* lsd);
        //void RestoreCanvasToBG(std::vector<char>* pixelMap, LogicalScreenDescriptor* lsd);
        //void RestoreToPrevState(std::vector<char>* pixMap, std::vector<char>* prevPixMap);
        
        //void LoadExtension(const ExtensionHeader& headerCheck);
        
        //// Debugging prints
        //void PrintDescriptor();
        //void PrintData();
        //void PrintSubBlockData(std::vector<uint8_t>* block);
//};

#endif // _GIF_IMAGE_DATA_H
