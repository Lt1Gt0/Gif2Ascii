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
        failure,
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
}

#endif // _GIF_META_H
