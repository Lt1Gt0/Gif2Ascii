#pragma once
#ifndef _IMAGE_META_H
#define _IMAGE_META_H

#include <stdint.h>
#include <vector>
#include "common.h"

#define EXTENSION_INTRODUCER        0x21
#define EXTENSION_TERMINATOR        0x00
#define IMAGE_DESCRIPTOR_SEPERATOR  0x2C
#define TRAILER                     0x3B

enum ImgDescMask {
    LocalColorTable = 7,
    Interlace       = 6,
    IMGSort         = 5, // Sort flag
    IMGSize         = 0, // Size of Local Color Table
};

enum ExtensionTypes { 
    PlainText       = 0x01,
    GraphicsControl = 0xF9,
    Comment         = 0xFE,
    Application     = 0xFF,
};

enum GCEMask {
    Disposal            = 2,
    UserInput           = 1,
    TransparentColor    = 0,
};

struct ImageDescriptor {
    uint8_t     Seperator;
    uint16_t    Left;
    uint16_t    Top;
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
} PACKED;

struct LocalColorTable {
    // TODO
} PACKED;

struct ImageDataHeader {
    uint8_t LZWMinimum;
    uint8_t FollowSize;
} PACKED;

struct SubBlock {
    uint8_t     FollowSize;
    uint8_t*    Data;
};

struct ExtensionHeader {
    uint8_t Introducer;
    uint8_t Label;
} PACKED;

struct GraphicsControlExtension {
    ExtensionHeader Header;
    uint8_t         BlockSize;

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
} PACKED;

struct PlainTextExtension {
    ExtensionHeader Header;
    uint8_t         BlockSize;
    uint8_t*        Data;
    uint8_t         Terminator;
};

struct ApplicationExtension {
    ExtensionHeader Header;
    uint8_t         BlockLength;
    uint8_t*        Identifier;
    uint8_t*        AuthenticationCode;
    uint8_t         Terminator;
};

struct CommentExtension {
    ExtensionHeader         Header;
    std::vector<uint8_t>    Data; 
};

struct ImageExtensions {
    GraphicsControlExtension*   GraphicsControl;
    PlainTextExtension*         PlainText;
    ApplicationExtension*       Application;
    CommentExtension*           Comment;
};

#endif // _IMAGE_META_H
