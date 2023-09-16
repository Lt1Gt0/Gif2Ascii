#pragma once
//#include "image.h"
#ifndef _GIF_HPP_
#define _GIF_HPP_

#include <vector>
#include <stdio.h>
#include <exception>
#include <fstream>
#include <string>
#include "gifmeta.hpp"
#include "pixel.hpp"
//#include "image.h"

namespace GIF
{
    class File
    {
        public:
            File(std::string filepath);
            ~File();

            void DumpInfo(std::string dumpPath);

            int Read(void* buf, size_t size, bool changePos = false, size_t offset = 0);

            DataStream::Stream              mDS;
            std::string                     mPath;
            std::ifstream                   mInStream;
            UNUSED Color*                   mGCT;
            // std::vector<void*>              mImageData;
            std::vector<std::vector<char>>  mFrameMap;
            std::size_t mFileSize;

        private:
            byte*  mLoadedData;
            std::size_t mFileOffset;
            std::streampos mCurrentFilePos;

            void Read();
            void ParseHeader();
            void ParseLSD();
            void ParseGCTD();
            void GenerateFrameMap();

            void CalculateFileSize();
            bool ValidHeader();
    };

    class Image 
    {            
         public: 
            Image();
            ~Image();
             
            std::string LoadData(File* const gif);
            void ReadDataSubBlock(File* const gif);
            void CheckExtensions(File* const gif);
    
            // Return the charstream given after decompression
            void UpdatePixelMap(UNUSED File* const gif, UNUSED const std::string& rasterData, std::vector<char> pixMap, UNUSED std::vector<char> prevPixMap);

            void DumpInfo(std::string dumpPath);
    
    //         Color*              mColorTable;
    //         ImageDescriptor     mDescriptor;
    //         ImageDataHeader     mDataHeader;
    //         ImageExtensions     mExtensions;
    //         std::vector<byte>   mData;
    //
            Data::Data*         mData;
            bool                mTransparent;
            byte                mTransparentColorIndex;
    //
         private:
            // Different Drawing behaviors based off Disposal Methods
            void DrawOverImage(File* const gif, const std::string& rasterData, std::vector<char> pixelMap);
            void RestoreCanvasToBG(File* const gif, std::vector<char> pixelMap);
            void RestoreToPrevState(std::vector<char> pixMap, std::vector<char> prevPixMap);
            
            void LoadExtension(File* const gif, const Data::ExtensionHeader& headerCheck);
            
            // Debugging prints
            void PrintDescriptor();
            void PrintData();
            void PrintSubBlockData(std::vector<byte>* block);

            byte mColorTableSize;
    };
}

#endif // _GIF_HPP_
