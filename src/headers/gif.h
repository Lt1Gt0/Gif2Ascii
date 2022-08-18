#pragma once
#ifndef _GIF_H
#define _GIF_H

#define UNUSED __attribute__((unused))

#include "gifmeta.h"

namespace GIF
{
    struct File {
        public: 
            const char*         mPath;
            FILE*               mFP;
            LSD                 mLSD;
            UNUSED GCTD         mGCTD;
            UNUSED Color*       mGCT;
            std::vector<void*>  mImageData;
            std::vector<char**> mFrameMap;

            File(const char* path);
            void OpenFile();
            Status ParseHeader();
            void Read();
            void DumpInfo(const char* path);

        private:
            size_t mSize;
            Header mHeader;

            Status ParseLSD();
            Status GenerateFrameMap();
    };

    struct Image {
        public:
            Color*             mColorTable;
            ImageDescriptor    mDescriptor;
            ImageDataHeader    mDataHeader;
            ImageExtensions    mExtensions;
            std::vector<byte>  mData;
            bool               mTransparent;
            byte               mTransparentColorIndex;

            Image(const File* gif) : mGIF(gif) {}
            std::string LoadData();
            void ReadDataSubBlocks();
            void CheckExtensions();
            void UpdatePixelMap(const std::string& rasterData, char** pixMap, UNUSED char** prevPixMap);
            
        private:
            const File* mGIF;
            void DrawOverImage(const std::string& rasterData, char** pixelMap);
            void RestoreCanvasToBG(char** pixelMap);
            void RestoreToPrevState(char** pixelMap, char** prevPixMap);

            void LoadExtension(const ExtensionHeader& headerCheck);
            
            // Debug Prints
            //void PrintDescriptor();
            //void printData();
            //void PrintSubBlockData();
    };

    Status Initialize(File* gif);
    void sigIntHandler(int sig);
}

#endif // _GIF_H
