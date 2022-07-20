#pragma once
#ifndef _GIF_META_H
#define _GIF_META_H

#include <stdint.h>
#include <vector>
#include "common.h"

#define GIF_MAGIC_0 "GIF87a"
#define GIF_MAGIC_1 "GIF89a"
#define NULL_COLOR  {0, 0, 0}
#define COLOR_SIZE  3

struct GifHeader {
    char Signature[3];
    char Version[3];
};


struct LogicalScreenDescriptor {
    uint16_t    Width;
    uint16_t    Height;

    /* Packed Bit Description
        1 : Global Color Table Flag
        2-4 : Color Resolution
        5 : Sort Flag
        6-8 : Global Color Table Size
    */
    uint8_t     Packed;
    uint8_t     BackgroundColorIndex;
    uint8_t     PixelAspectRatio;
} PACKED;

struct GlobalColorTableDescriptor {
    uint8_t     SizeInLSD;
    uint16_t    NumberOfColors;
    uint16_t    ByteLegth;
} PACKED;

struct Color {
    uint8_t Red;
    uint8_t Blue;
    uint8_t Green;
};


namespace LSDMask 
{
    enum {
        GlobalColorTable    = 7,
        ColorResolution     = 4,
        Sort             = 3, // Sort flag
        Size             = 0, // Size of Global Color Table
    };
}

#endif // _GIF_META_H
