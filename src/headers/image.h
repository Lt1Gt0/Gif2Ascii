#pragma once
#ifndef _GIF_IMAGE_DATA_H
#define _GIF_IMAGE_DATA_H

#define PACKED __attribute__((packed))

#include "gif.h"
#include "common.h"
#include <vector>

namespace Image
{
    namespace Meta
    {
        constexpr byte EXTENSION_INTRODUCER         {0x21};
        constexpr byte EXTENSION_TERMINATOR         {0x00};
        constexpr byte IMAGE_DESCRIPTOR_SEPERATOR   {0x2C};
        constexpr byte TRAILER                      {0x3B};

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

        struct PACKED Descriptor {
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

        struct PACKED DataHeader {
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

        struct Extensions {
            GCE                     GraphicsControl;
            PlainTextExtension      PlainText;
            ApplicationExtension    Application;
            CommentExtension        Comment;
        };
    }

    struct Image : private GIF::File {
        public:
            GIF::Color*         mColorTable;
            Meta::Descriptor    mDescriptor;
            Meta::DataHeader    mDataHeader;
            Meta::Extensions    mExtensions;
            std::vector<byte>   mData;
            bool                mTransparent;
            byte                mTransparentColorIndex;

            // TODO
            Image(GIF::File* gif);
            
            // TODO
    };
}

//#include "imagemeta.h"
//#include "gifmeta.h"
//#include <stdio.h>
//#include <stdint.h>
//#include <string>
//#include <vector>

//class Image 
//{            
    //public:
        //Color* mColorTable;
        //ImageDescriptor mDescriptor;
        //ImageDataHeader mHeader;
        //ImageExtensions mExtensions;
        //std::vector<uint8_t> mData;

        //bool mTransparent;
        //uint8_t mTransparentColorIndex;
        
    //public:
        //Image(FILE* _fp, Color* _colortable, uint8_t _colorTableSize);
        
        //std::string LoadImageData();
        //void ReadDataSubBlocks();
        //void CheckExtensions();

        //// Return the charstream given after decompression
        //void UpdatePixelMap(std::vector<char>* pixMap, std::vector<char>* prevPixMap, std::string* rasterData, LogicalScreenDescriptor* lsd);

    //private:
        //FILE* mFile;
        //uint8_t mColorTableSize;
    
    //private:
        //// Different Drawing behaviors based off Disposal Methods
        //void DrawOverImage(std::string* rasterData, std::vector<char>* pixelMap, LogicalScreenDescriptor* lsd);
        //void RestoreCanvasToBG(std::vector<char>* pixelMap, LogicalScreenDescriptor* lsd);
        //void RestoreToPrevState(std::vector<char>* pixMap, std::vector<char>* prevPixMap);
        
        //void LoadExtension(const ExtensionHeader& headerCheck);
        
        //// Debugging prints
        //void PrintDescriptor();
        //void PrintData();
        //void PrintSubBlockData(std::vector<uint8_t>* block);
//};

#endif // _GIF_IMAGE_DATA_H
