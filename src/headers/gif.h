#pragma once
//#include "image.h"
#ifndef _GIF_H
#define _GIF_H

#include <vector>
#include <stdio.h>
#include <exception>
#include "gifmeta.h"
//#include "image.h"

#define UNUSED __attribute__((unused))

namespace GIF
{
    class File
    {
        public:
            File(const char* filepath);
            ~File();

            void Read();
            void DumpInfo(const char* dumpPath);

            const char*                     mPath;
            FILE*                           mFP;
            LSD                             mLSD;
            UNUSED GCTD                     mGCTD;
            UNUSED Color*                   mGCT;
            std::vector<void*>              mImageData;
            std::vector<std::vector<char>>  mFrameMap;

        private:
            size_t mFileSize;
            Header mHeader;

            Status ParseHeader();
            Status ParseLSD();
            Status GenerateFrameMap();
    };

    class Image 
    {            
        public: 
            Image();
            ~Image();
            
            std::string LoadData(File* const gif);
            void ReadDataSubBlocks(File* const gif);
            void CheckExtensions(File* const gif);

            // Return the charstream given after decompression
            void UpdatePixelMap(UNUSED File* const gif, UNUSED const std::string& rasterData, std::vector<char> pixMap, UNUSED std::vector<char> prevPixMap);

            Color*              mColorTable;
            ImageDescriptor     mDescriptor;
            ImageDataHeader     mDataHeader;
            ImageExtensions     mExtensions;
            std::vector<byte>   mData;
            bool                mTransparent;
            byte                mTransparentColorIndex;

        private:
            // Different Drawing behaviors based off Disposal Methods
            void DrawOverImage(File* const gif, const std::string& rasterData, std::vector<char> pixelMap);
            void RestoreCanvasToBG(File* const gif, std::vector<char> pixelMap);
            void RestoreToPrevState(std::vector<char> pixMap, std::vector<char> prevPixMap);
            
            void LoadExtension(File* const gif, const ExtensionHeader& headerCheck);
            
            // Debugging prints
            //void PrintDescriptor();
            //void PrintData();
            //void PrintSubBlockData(std::vector<byte>* block);

            byte mColorTableSize;
        
    };
}

#endif // _GIF_H
