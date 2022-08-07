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

            File(const char* path) : mPath(path) {}
            void OpenFile();
            Status ParseHeader();
            void Read();
            
            void DumpInfo(const char* path);

        private:
            FILE*           mFP;
            Header          mHeader;
            LSD             mLSD;
            UNUSED GCTD     mGCTD;
            UNUSED Color*   mGCT;

            Status ParseLSD();
            Status ParseGCT();
            Status GenerateFrameMap();
            
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
