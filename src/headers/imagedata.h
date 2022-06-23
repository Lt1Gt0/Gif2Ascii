#pragma once
#ifndef _GIF_IMAGE_DATA_H
#define _GIF_IMAGE_DATA_H

#include "gifheaders.h"
#include <stdio.h>
#include <stdint.h>

#include <vector>

namespace ImageData 
{
    enum ImgDescMask {
        LocalColorTable = 7,
        Interlace       = 6,
        IMGSort         = 5, // Sort flag
        IMGSize         = 0, // Size of Local Color Table
    };

    struct ImageDescriptor {
        uint8_t     Seperator;
        uint16_t    Left;
        uint16_t    Right;
        uint16_t    Width;
        uint16_t    Height;

        /* Packed Bit Description
            1 : Local Color Table Flag
            2 : Interlace Flag
            3 : Sort Flag
            4-5 : Reserved
            6-8 : Local Color Table Size
        */
        uint8_t     Packed;
    } __attribute__((packed));

    // struct LocalColorTable {

    // } __attribute__((packed));

    struct ImageDataHeader {
        uint8_t LZWMinimum;
        uint8_t FollowSize;
    } __attribute__((packed));

    class Image 
    {
        public:
            Image();
            void PrintData();
            ImageDescriptor* descriptor;
            ImageDataHeader* header;
            std::vector<std::vector<uint8_t>> subBlocks;

            void ReadDataSubBlocks(FILE* file);
        private:
            void PrintDescriptor();
            void PrintSubBlockData(std::vector<uint8_t> block);
    };
};

#endif // _GIF_IMAGE_DATA_H