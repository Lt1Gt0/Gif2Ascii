#pragma once
#ifndef _GIF_HEADERS_H
#define _GIF_HEADERS_H

#include <stdint.h>

namespace GIF_Headers
{
    #define GIF_MAGIC_0 "GIF87a"
    #define GIF_MAGIC_1 "GIF89a"

    #define EXTENSION_INTRODUCER        0x21
    #define EXTENSION_TERMINATOR        0x00
    #define IMAGE_DESCRIPTOR_SEPERATOR  0x2C
    #define TRAILER                     0x3B

    enum ExtensionTypes { 
        PlainText       = 0x01,
        GraphicsControl = 0xF9,
        Comment         = 0xFE,
        Application     = 0xFF,
    };

    struct Header {
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

    struct ColorTable {
        uint8_t Red;
        uint8_t Green;
        uint8_t Blue;
    } __attribute__((packed));    
    
    struct DataSubBlock {
        uint8_t     Size;
        uint8_t*    Values;        
    };

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

    struct ImageData {
        uint8_t LZWMinimum;
        uint8_t FollowSize;
    } __attribute__((packed));
    
    struct ExtensionHeader {
        uint8_t Introducer;
        uint8_t Label;
    } __attribute__((packed));

    enum GCEMask {
        Disposal            = 2,
        UserInput           = 1,
        TransparentColor    = 0,
    };

    struct GraphicsControlExtension {
        ExtensionHeader header;

        /* Packed Bit Description
            1-3 : Reserved
            4-6 : Disposal Method
            7 : User Input Flag
            8 : Transparent Color Flag
        */
        uint8_t         Packed;

        uint16_t        DelayTime;
        uint8_t         TransparentColorIndex;
        uint8_t         BlockTerminator; // Always 0x00
    } __attribute__((packed));

    struct PlainTextExtension {
        ExtensionHeader header;
        uint8_t         BlockSize;
        uint8_t*        data;
    } __attribute__((packed));

    struct ApplicationExtension {
        ExtensionHeader header;
        uint8_t         BlockLength;
        uint8_t*        Identifier;
        uint8_t*        AuthenticationCode;
        // DataSubBlock    dataSubBlock;
        uint8_t         Terminator;
    };

    struct CommentExtension {
        ExtensionHeader header;
        uint8_t*           data;
    } __attribute__((packed));


}

#endif // _GIF_HEADERS_H