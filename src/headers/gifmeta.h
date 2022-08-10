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

    constexpr byte EXTENSION_INTRODUCER         {0x21};
    constexpr byte EXTENSION_TERMINATOR         {0x00};
    constexpr byte IMAGE_DESCRIPTOR_SEPERATOR   {0x2C};
    constexpr byte TRAILER                      {0x3B};

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


    struct PACKED ImageDescriptor {
        byte Seperator;
        word Left;
        word Top;
        word Width;
        word Height;

        /*
            Packed Bit Description
            1 : Local Color Table Flag
            2 : Interlace Flag
            3 : Sort Flag
            4-5 : Reserved
            6-8 : Local Color Table Size
        */
        byte Packed;
    };

    // Local Color Table
    struct PACKED LCT {
        // TODO
    };

    struct PACKED ImageDataHeader {
        byte LZWMinimum;
        byte FollowSize;
    }; 

    struct SubBlock {
        byte    FollowSize;
        byte*   Data;
    };

    struct PACKED ExtensionHeader {
        byte            Introducer;
        ExtensionLabel  Label;
    };

    // Graphics Control Extension
    struct PACKED GCE {
        ExtensionHeader Header;
        byte            BlockSize;

        /*
            Packed Bit Description
            1-3 : Reserved
            4-6 : Disposal Method
            7 : User Input Flag
            8 : Transparent Color Flag
        */
        byte            Packed;
        word            DelayTime;
        byte            TransparentColorIndex;
        byte            BlockTerminator; // Always 0x00
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
        ExtensionHeader     Header;
        std::vector<byte>   Data; 
    };

    struct ImageExtensions {
        GCE                     GraphicsControl;
        PlainTextExtension      PlainText;
        ApplicationExtension    Application;
        CommentExtension        Comment;
    };
}

#endif // _GIF_META_H
