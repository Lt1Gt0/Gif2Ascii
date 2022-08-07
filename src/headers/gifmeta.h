#pragma once
#ifndef _GIF_META_H
#define _GIF_META_H

#include <stdint.h>
#include <stdio.h>
#include <vector>
#include "common.h"

#define PACKED __attribute__((packed))

namespace GIF
{
    constexpr byte GIF_SIGNATURE[3] {'G', 'I', 'F'};
    constexpr byte GIF_87A[3]       {'8', '7', 'a'};
    constexpr byte GIF_89A[3]       {'8', '9', 'a'};

    enum class Status {
        success,
        failure
    };

    enum class LSDMask : byte {
        GlobalColorTable    = 0x7,
        ColorResolution     = 0x4,
        Sort                = 0x3, // Sort flag
        Size                = 0x0, // Size of Global Color Table
    };

    struct PACKED Header {
        byte signature[3];
        byte version[3];
    };

    // Logical Screen Descriptor
    struct PACKED LSD {
        word width;
        word height;
        byte packed;
        byte backgroundColorIndex;
        byte pixelAspectRatio;
    };

    struct Color {
        byte Red;
        byte Green;
        byte Blue;
    };

    // Global Color Table Descriptor
    struct PACKED GCTD {
        byte sizeInLSD;
        word colorCount;
        word byteLength;
    };

    // Local Color Table
    struct LCT {
        std::vector<byte> table;
    };
}

//#include <stdint.h>
//#include <vector>

//constexpr char gifSignature[3]  {'G', 'I', 'F'};
//constexpr char gif87a[3]        {'8', '7', 'a'};
//constexpr char gif89a[3]        {'8', '9', 'a'};
//#define NULL_COLOR  {0, 0, 0}
//#define COLOR_SIZE  3

//enum class LSDMask : uint8_t {
    //GlobalColorTable    = 0x07,
    //ColorResolution     = 0x04,
    //Sort                = 0x03, // Sort flag
    //Size                = 0x00, // Size of Global Color Table
//};

//struct GifHeader {
    //char Signature[3];
    //char Version[3];
//};

//struct LogicalScreenDescriptor {
    //uint16_t    Width;
    //uint16_t    Height;

    //[> Packed Bit Description
        //1 : Global Color Table Flag
        //2-4 : Color Resolution
        //5 : Sort Flag
        //6-8 : Global Color Table Size
    //*/
    //uint8_t     Packed;
    //uint8_t     BackgroundColorIndex;
    //uint8_t     PixelAspectRatio;
//} __attribute__((packed));

//struct GlobalColorTableDescriptor {
    //uint8_t     SizeInLSD;
    //uint16_t    NumberOfColors;
    //uint16_t    ByteLegth;
//} __attribute__((packed));

//struct Color {
    //uint8_t Red;
    //uint8_t Blue;
    //uint8_t Green;
//};

#endif // _GIF_META_H
