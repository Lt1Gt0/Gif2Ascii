#pragma once
#ifndef _GIF_HEADERS_H
#define _GIF_HEADERS_H

#include <stdint.h>

namespace GIF_Headers
{
    #define GIF_MAGIC_0 "GIF87a"
    #define GIF_MAGIC_1 "GIF89a"

    #define EXTENSION_INTRODUCER_MAGIC  0x21
    #define IMAGE_DESCRIPTOR_SEPERATOR  0x2C
    #define TRAILER                     0x3B

    struct Header {
        char Signature[3];
        char Version[3];
    } __attribute__((packed));

    enum LSDFlags {
        GlobalColorTable = 7,
        ColorResolution = 4,
        Sort = 3,
        Size = 0,
    };

    struct LogicalScreenDescriptor {
        uint16_t    Width;
        uint16_t    Height;

        /* GCTF | RES | RES | RES | SORT | SIZE | SIZE | SIZE */
        uint8_t     packed;
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

    /* Optional for GIF89a */
    struct GraphicsControlExtension {
        uint8_t ExtensionIntroducer;
        uint8_t GraphicControlLabel;
        
        /* Packed Field */
        uint8_t resv0           : 3;
        uint8_t DisposalMethod  : 3;
        uint8_t UserInputFlag   : 1;
        uint8_t TransparentFlag : 1;

        uint16_t DelayTime;
        uint8_t TransparentColorIndex;
        uint8_t BlockTerminator; // Always 0x00
    } __attribute__((packed));

    struct ImageDescriptor {
        uint8_t     ImageSeperator;
        uint16_t    ImageLeft;
        uint16_t    ImageRight;
        uint16_t    ImageWidth;
        uint16_t    ImageHeight;

        /* Packed Field */
        uint8_t     LocalColorTableFlag : 1;
        uint8_t     InterLaceFlag       : 1;
        uint8_t     SortFlag            : 1;
        uint8_t     resv1               : 2;
        uint8_t     LocalColorTableSize : 3;
    } __attribute__((packed));

    // struct LocalColorTable {

    // } __attribute__((packed));

    // struct ImageData {

    // } __attribute__((packed));

    // struct PlainTextExtension {

    // } __attribute__((packed));

    // struct ApplicationExtension {

    // } __attribute__((packed));

    // struct CommentExtension {

    // } __attribute__((packed));

    // struct Trailer {

    // } __attribute__((packed));
}

#endif // _GIF_HEADERS_H