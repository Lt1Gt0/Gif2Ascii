#pragma once
#ifndef _GIF_H
#define _GIF_H

#define UNUSED __attribute__((unused))

#include "gifmeta.h"

namespace GIF
{
    struct File {
        public: 
            const char* mPath;

            File();
            File(const char* path) : mPath(path) {}
            void OpenFile();
            Status ParseHeader();
            void Read();
            
            void DumpInfo(const char* path);

            static void sigIntHandler(int sig);

        protected:
            FILE*           mFP;
            Header          mHeader;
            LSD             mLSD;
            UNUSED GCTD     mGCTD;
            UNUSED Color*   mGCT;
            char**          mPixelMap;

            Status ParseLSD();
            Status GenerateFrameMap();
    };

    /* TODO */
    struct Image {
        public:
            Color*             mColorTable;
            ImageDescriptor    mDescriptor;
            ImageDataHeader    mDataHeader;
            ImageExtensions    mExtensions;
            std::vector<byte>       mData;
            bool                    mTransparent;
            byte                    mTransparentColorIndex;

            // TODO
            Image();

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

            void LoadExtension(const ExtensionHeader& headerCheck);
            
            // Debug Prints
            //void PrintDescriptor();
            //void printData();
            //void PrintSubBlockData();
    };

    Status Initialize(File* gif);
}

//#include <vector>
//#include <stdio.h>
//#include <exception>
//#include "gifmeta.h"
//#include "image.h"

//class GIF 
//{
    //public:
        //FILE* mFile;
        
        //GifHeader mHeader;
        //LogicalScreenDescriptor mLsd;
        //GlobalColorTableDescriptor mGctd;
        //std::vector<Image> mImageData;
        //Color* mColorTable; // If the flag is present then the gct will be filled
        //std::vector<std::vector<char>> mFrameMap;

    //public:
        //GIF(const char* _filepath);
       
        /** 
         * Read each header of the file into their respective members
         */ 
        //void Read();
        //static void SigIntHandler(int sig);

    //private:
        //size_t mFilesize;
        //bool mHeaderInitialized;
        //bool mLSDInitialized;
        //bool mFrameMapInitialized;
        //bool mFrameOutOfBounds;

        //std::vector<char> mPixelMap;
        //std::vector<char> mPrevPixelMap;

    //private:
        /**
         * Load GIF File header into mHeader
         *
         * @return NONE
         */
        //void LoadHeader();

        /**
         * Check if mHeader is a valid header according
         * to GIF87a or GIF98a standard (does not differentiate between the two)
         *
         * @return True if the header is valid, false if otherwise
         */
        //bool ValidHeader();

        /**
         * Load GIF Logical Screen Descriptor from mFile
         *
         * @return NONE
         */ 
        //void LoadLSD();

        /**
         * Generate a pixel map for each frame in mFile
         *
         * @return NONE
         */
        //void GenerateFrameMap();
        
        //// Debug Prints
        //void PrintHeaderInfo();
        //void PrintColorTable();
//};

#endif // _GIF_H
