#pragma once
#ifndef _GIF_META_H
#define _GIF_META_H

#include <stdint.h>
#include <vector>

#define GIF_MAGIC_0 "GIF87a"
#define GIF_MAGIC_1 "GIF89a"

struct GifHeader {
    char Signature[3];
    char Version[3];
} __attribute__((packed));

enum LSDMask {
    GlobalColorTable    = 7,
    ColorResolution     = 4,
    LSDSort             = 3, // Sort flag
    LSDSize             = 0, // Size of Global Color Table
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
} __attribute__((packed));

struct GlobalColorTableDescriptor {
    uint8_t     SizeInLSD;
    uint16_t    NumberOfColors;
    uint16_t    ByteLegth;
} __attribute__((packed));

#endif // _GIF_META_H