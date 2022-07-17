#pragma once
#ifndef _GIF_H
#define _GIF_H

#include <vector>
#include <stdio.h>
#include "gifmeta.h"
#include "image.h"

class GIF 
{
    public:
        FILE* mFile;
        
        GifHeader* mHeader;
        LogicalScreenDescriptor* mLsd;
        GlobalColorTableDescriptor* mGctd;
        std::vector<Image> mImageData;
        Color* mColorTable; // If the flag is present then the gct will be filled
        std::vector<std::vector<char>> mFrameMap;

    public:
        GIF(FILE* _fp);
       
        /** 
         * Read each header of the file into their respective members
         */ 

        void Read();

        /**
         * Loop through each frame in the loaded frame map
         *
         * @return NONE
         */
        void LoopFrames();

    private:
        size_t mFilesize;
        bool mHeaderInitialized;
        bool mLSDInitialized;
        bool mFrameMapInitialized;
        
    private:
        /**
         * Load GIF File header into mHeader
         *
         * @return NONE
         */
        void LoadHeader();

        /**
         * Check if mHeader is a valid header according
         * to GIF87a or GIF98a standard (does not differentiate between the two)
         *
         * @return True if the header is valid, false if otherwise
         */
        bool ValidHeader();

        /**
         * Load GIF Logical Screen Descriptor from mFile
         *
         * @return NONE
         */ 
        void LoadLSD();

        /**
         * Generate a pixel map for each frame in mFile
         *
         * @return NONE
         */
        void GenerateFrameMap();
        
        // Debug Prints
        void PrintHeaderInfo();
        void PrintColorTable();
};

#endif // _GIF_H
