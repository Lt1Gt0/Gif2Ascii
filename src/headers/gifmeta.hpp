#pragma once
#ifndef _GIF_META_HPP_
#define _GIF_META_HPP_

#include <stdint.h>
#include <vector>
#include <stdio.h>
#include "utils/types.hpp"
#include "utils/definitions.hpp"

namespace GIF
{
    constexpr const char* GIF_SIGNATURE {"GIF"};
    constexpr const char* GIF_87A       {"87a"};
    constexpr const char* GIF_89A       {"89a"};
    
    constexpr byte EXTENSION_INTRODUCER         {0x21};
    constexpr byte EXTENSION_TERMINATOR         {0x00};
    constexpr byte IMAGE_DESCRIPTOR_SEPERATOR   {0x2C};
    constexpr byte TRAILER                      {0x3B};

    namespace LogicalScreen
    {
        enum class Mask : byte {
            GlobalColorTable    = 0x07,
            ColorResolution     = 0x04,
            Sort                = 0x03, // Sort flag
            Size                = 0x00, // Size of Global Color Table
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
    }

    namespace Data
    {
        enum class ExtensionLabel : byte {
            PlainText       = 0x01,
            GraphicsControl = 0xF9,
            Comment         = 0xFE,
            Application     = 0xFF,
        };

        struct PACKED ExtensionHeader {
            byte         introducer;
            ExtensionLabel  label;
        };

        namespace Graphic
        {
            // Graphics Control Extension
            enum class GCEMask : byte {
                Disposal            = 2,
                UserInput           = 1,
                TransparentColor    = 0 
            };

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

            enum class ImgDescMask : byte {
                LocalColorTable = 7,
                Interlace       = 6,
                IMGSort         = 5, // Sort flag
                IMGSize         = 0, // Size of Local Color Table
            };

            struct PACKED ImageDescriptor {
                byte    seperator;
                word    left;
                word    top;
                word    width;
                word    height;

                /* Packed Bit Description
                    1 : Local Color Table Flag
                    2 : Interlace Flag
                    3 : Sort Flag
                    4-5 : Reserved
                    6-8 : Local Color Table Size
                */
                byte    packed;
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

            struct PlainTextExtension {
                ExtensionHeader header;
                byte            blockSize;
                word            textGridLeft;
                word            textGridTop;
                word            textGridWidth;
                word            textGridHeight;
                byte            cellWidth;
                byte            cellHeight;
                byte            textFgColorIndex;
                byte            textBgColorIndex;
                byte*           data;
                byte            terminator;
            };

            struct PACKED Graphic {
                ImageDescriptor imgDesc;
                LCT lct;
                PlainTextExtension pte;

                GCE gce;
            };
        }

        // <Special-Purpose Block> ::= Application Extension | Comment Extension
        namespace Special
        {
            struct ApplicationExtension {
                ExtensionHeader header;
                byte            blockLength;
                byte*           identifier;
                byte*           authenticationCode;
                byte*           data;
                byte            terminator;
            };

            struct CommentExtension {
                ExtensionHeader     header;
                std::vector<byte>   data; 
                byte                temrinator;
            };

            struct Special {
                ApplicationExtension application;
                CommentExtension comment;
            };
        }
        
        struct Data {
            Graphic::Graphic graphic;
            Special::Special special;
        };
    }

    namespace DataStream
    {
        struct PACKED Header {
            char signature[3];
            char version[3];
        };

        struct Stream {
            Header header;

            LogicalScreen::LSD lsd;
            LogicalScreen::GCTD gctd;
            std::vector<Data::Data> data;
            byte trailer; 
        };
    }
}

#endif // _GIF_META_HPP_
