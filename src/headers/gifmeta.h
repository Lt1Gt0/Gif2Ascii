#pragma once
#ifndef _GIF_META_H
#define _GIF_META_H

#include <stdint.h>
#include <vector>
#include "common.h"

#define PACKED __attribute__((packed))

namespace GIF
{
    constexpr const char* GIF_SIGNATURE {"GIF"};
    constexpr const char* GIF_87A       {"87a"};
    constexpr const char* GIF_89A       {"89a"};
    
    constexpr byte EXTENSION_INTRODUCER         {0x21};
    constexpr byte EXTENSION_TERMINATOR         {0x00};
    constexpr byte IMAGE_DESCRIPTOR_SEPERATOR   {0x2C};
    constexpr byte TRAILER                      {0x3B};

    enum class Status {
        success,
        failure
    };

    enum class LSDMask : byte {
        GlobalColorTable    = 0x07,
        ColorResolution     = 0x04,
        Sort                = 0x03, // Sort flag
        Size                = 0x00, // Size of Global Color Table
    };

    enum class ImgDescMask : byte {
        LocalColorTable = 7,
        Interlace       = 6,
        IMGSort         = 5, // Sort flag
        IMGSize         = 0, // Size of Local Color Table
    };

    enum class ExtensionLabel : byte {
        PlainText       = 0x01,
        GraphicsControl = 0xF9,
        Comment         = 0xFE,
        Application     = 0xFF,
    };

    enum class GCEMask : byte {
        Disposal            = 2,
        UserInput           = 1,
        TransparentColor    = 0 
    };

    struct PACKED Header {
        char signature[3];
        char version[3];
    };

    // Logical Screen Descriptor
    struct PACKED LSD {
        word    width;
        word    height;

        /* Packed Bit Description
            1 : Global Color Table Flag
            2-4 : Color Resolution
            5 : Sort Flag
            6-8 : Global Color Table Size
        */
        byte    packed;
        byte    backgroundColorIndex;
        byte    pixelAspectRatio;
    };

    // Global Color Table Descriptor
    struct PACKED GCTD {
        byte    sizeInLSD;
        word    colorCount;
        word    byteLength;
    };

    struct Color {
        byte Red;
        byte Blue;
        byte Green;
    };

    struct PACKED ImageDescriptor {
        byte    Seperator;
        word    Left;
        word    Top;
        word    Width;
        word    Height;

        /* Packed Bit Description
            1 : Local Color Table Flag
            2 : Interlace Flag
            3 : Sort Flag
            4-5 : Reserved
            6-8 : Local Color Table Size
        */
        byte    Packed;
    };

    // Local Color Table
    struct PACKED LCT {
        // TODO
    };

    struct PACKED ImageDataHeader {
        byte lzwMinimum;
        byte followSize;
    }; 

    struct SubBlock {
        byte    followSize;
        byte*   data;
    };

    struct PACKED ExtensionHeader {
        byte         introducer;
        ExtensionLabel  label;
    };

    // Graphics Control Extension
    struct PACKED GCE {
        ExtensionHeader header;
        byte            blockSize;

        /* Packed Bit Description
            1-3 : Reserved
            4-6 : Disposal Method
            7 : User Input Flag
            8 : Transparent Color Flag
        */
        byte            packed;

        word            delayTime;
        byte            transparentColorIndex;
        byte            blockTerminator; // Always 0x00
    };

    struct PlainTextExtension {
        ExtensionHeader header;
        byte            blockSize;
        byte*           data;
        byte            terminator;
    };

    struct ApplicationExtension {
        ExtensionHeader header;
        byte            blockLength;
        byte*           identifier;
        byte*           authenticationCode;
        byte            terminator;
    };

    struct CommentExtension {
        ExtensionHeader     header;
        std::vector<byte>   data; 
    };

    struct ImageExtensions {
        GCE                     graphicsControl;
        PlainTextExtension      plainText;
        ApplicationExtension    application;
        CommentExtension        comment;
    };
}

#endif // _GIF_META_H
