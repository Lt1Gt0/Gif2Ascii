#include "image.hpp"

#include <cstdint>
#include <stdio.h>
#include <unordered_map>
#include "lzw.hpp"
#include "utils/logger.hpp"
#include "utils/error.hpp"

Image::Image(FILE* _fp, Color* _colortable, uint8_t _colorTableSize)
{
    this->mFile = _fp;
    this->mColorTable = _colortable;
    this->mColorTableSize = _colorTableSize;

    this->mDescriptor = {};
    this->mHeader = {};
    this->mExtensions = {};
    this->mData = std::vector<uint8_t>();
}

std::string Image::LoadImageData()
{
    logger.Log(TRACE, "Loading image data");

    // Load the Image Descriptor into memory
    fread(&this->mDescriptor, sizeof(uint8_t), sizeof(ImageDescriptor), this->mFile);

    // TODO:
    //Add support for LCT in GIFS that require it
    if ((this->mDescriptor.Packed >> (uint8_t)ImgDescMask::LocalColorTable) & 0x1)
        logger.Log(DEBUG, "Loading Local Color Table");
    else
        logger.Log(DEBUG, "Local Color Table flag not set");

    // Load the image header into memory
    fread(&this->mHeader, sizeof(uint8_t), sizeof(ImageDataHeader), this->mFile); // Only read 2 bytes of file steam for LZW min and Follow Size 
    ReadDataSubBlocks();

    // Get the raster data from the image frame by decompressing the data block from the gif
    std::string rasterData = LZW::Decompress(this->mHeader, this->mColorTableSize, this->mData);
    return rasterData;
}

void Image::ReadDataSubBlocks()
{
    logger.Log(TRACE, "Reading data subblocks");

    uint8_t nextByte = 0;
    int followSize = this->mHeader.FollowSize;
    
    while (followSize--) {
        fread(&nextByte, sizeof(uint8_t), 1, this->mFile);
        this->mData.push_back(nextByte);
    }

    fread(&nextByte, sizeof(uint8_t), 1, this->mFile);

    while (true) {
        // Check for the end of sub block
        if (!nextByte)
            break;
        
        followSize = nextByte;
        while (followSize--) {
            fread(&nextByte, sizeof(uint8_t), 1, this->mFile);
            this->mData.push_back(nextByte);
        }

        fread(&nextByte, sizeof(uint8_t), 1, this->mFile);
    }

    // Print out the compressed stream of data 
    for (uint8_t d : this->mData) {
        fprintf(stdout, "%X ", d);
    }

    // Debug::Print("\n");
}

void Image::CheckExtensions()
{
    logger.Log(TRACE, "Checking for extensions");

    // Load a dummy header into memory
    // Allocate space in memory for an extension header
    ExtensionHeader extensionCheck = {};

    // Continue to loop until the next byte is not an extension introducer
    while (true) {
        fread(&extensionCheck, sizeof(uint8_t), sizeof(ExtensionHeader), this->mFile);

        // If the dummy header contains an introducer for a extension, load the extension type
        if (extensionCheck.Introducer == EXTENSION_INTRODUCER) {
            LoadExtension(extensionCheck);
        } else {
            fseek(this->mFile, -2, SEEK_CUR);
            return;
        }
    }
}

void Image::LoadExtension(const ExtensionHeader& headerCheck)
{
    logger.Log(TRACE, "Load Extensions");

    // If the header has a valid label for an extension, start each one
    // by loading the struct into memory and set its header to the one that was passed in
    fseek(this->mFile, -2, SEEK_CUR);

    switch (headerCheck.Label) {
        case ExtensionLabel::PlainText:
        {
            logger.Log(DEBUG, "Loading plain text extension");

            // Load Header
            this->mExtensions.PlainText = {};
            fread(&this->mExtensions.PlainText.Header, sizeof(uint8_t), sizeof(ExtensionHeader), this->mFile);

            // Load the block size into the struct and load the data of that size into the data buffer
            fread(&this->mExtensions.PlainText.BlockSize, sizeof(uint8_t), 1, this->mFile);

            this->mExtensions.PlainText.Data = new uint8_t[this->mExtensions.PlainText.BlockSize];
            fread(&this->mExtensions.PlainText.Data, sizeof(uint8_t), this->mExtensions.PlainText.BlockSize, this->mFile);

            logger.Log(DEBUG, "End of plain text extension");
            break;
        }
        case ExtensionLabel::GraphicsControl:
        {
            logger.Log(DEBUG, "Loading graphics control extension");

            // Load The entire Graphic Control Extension
            this->mExtensions.GraphicsControl = {};
            fread(&this->mExtensions.GraphicsControl, sizeof(uint8_t), sizeof(GraphicsControlExtension), this->mFile);
            
            // Check for transparency
            if ((this->mExtensions.GraphicsControl.Packed >> (uint8_t)GCEMask::TransparentColor) & 0x01) {
                this->mTransparent = true;
                this->mTransparentColorIndex = this->mExtensions.GraphicsControl.TransparentColorIndex;

                logger.Log(DEBUG, "Transparent flag set in image");
                logger.Log(DEBUG, "Tranparent Color Index: ");
            } else {
                logger.Log(DEBUG, "Transparent flag not set"); 
            }
            
            logger.Log(DEBUG, "End of graphics control extension");
            break;
        }
        case ExtensionLabel::Comment:
        {
            logger.Log(DEBUG, "Loading comment extension");

            // Load Header
            this->mExtensions.Comment = {};
            fread(&this->mExtensions.Comment.Header, sizeof(uint8_t), sizeof(ExtensionHeader), this->mFile);

            // Read into the data section until a null terminator is hit
            uint8_t nextByte = 0;
            for (int i = 0; nextByte != 0x00; i++) {
                fread(&nextByte, sizeof(uint8_t), 1, this->mFile);
                this->mExtensions.Comment.Data.push_back(nextByte);
            }

            logger.Log(DEBUG, "End of comment extension");
            break;
        }
        case ExtensionLabel::Application:
        {
            logger.Log(DEBUG, "Loading application extension");

            // Load Header
            this->mExtensions.Application = {};
            fread(&this->mExtensions.Application.Header, sizeof(uint8_t), sizeof(ExtensionHeader), this->mFile);

            // Load the Block Length
            fread(&this->mExtensions.Application.BlockLength, sizeof(uint8_t), 1, this->mFile);

            // Load Application Identifier
            fread(&this->mExtensions.Application.Identifier, sizeof(uint8_t), this->mExtensions.Application.BlockLength, this->mFile);
            
            // Load the authentication code
            uint8_t tmp = 0;
            fread(&tmp, sizeof(uint8_t), 1, this->mFile);
            fread(&this->mExtensions.Application.AuthenticationCode, sizeof(uint8_t), tmp, this->mFile);

            // Check if the next byte in the file is the terminator
            fread(&tmp, sizeof(uint8_t), 1, this->mFile);
            if (!tmp)
                logger.Log(DEBUG, "End of application extension");

            break;
        }
        default:
        {
            logger.Log(DEBUG, "Recived invalid extension type [");
            fseek(this->mFile, 2, SEEK_CUR); // Restore the file position to where it was after reading header
            break;
        }
    }
}

void Image::UpdatePixelMap(std::vector<char>* pixMap, std::vector<char>* prevPixMap, std::string* rasterData, LogicalScreenDescriptor* lsd)
{
    logger.Log(TRACE, "Updating pixel map");

    // Because each gif can have a different disposal method for different frames (according to GIF89a)
    // it is best to handle each disposal method instread of printing the decompressed codestream directly
    int disposalMethod = ((this->mExtensions.GraphicsControl.Packed >> (uint8_t)GCEMask::Disposal) & 0x07);
    switch (disposalMethod) {
    case 0:
        break;
    case 1:
        DrawOverImage(rasterData, pixMap, lsd);
        break;
    case 2:
        RestoreCanvasToBG(pixMap, lsd);
        break;
    case 3:
        RestoreToPrevState(pixMap, prevPixMap);
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        break;
    default:
        error(Severity::medium, "Image:", "undefined disposal method -", disposalMethod);
        break;
    }
}

void Image::DrawOverImage(std::string* rasterData, std::vector<char>* pixelMap, LogicalScreenDescriptor* lsd)
{
    logger.Log(TRACE, "Drawing over image");

    int offset = 0;
    int currentChar = 0;
    for (int row = 0; row < this->mDescriptor.Height; row++) {
        for (int col = 0; col < this->mDescriptor.Width; col++) {
            if ((size_t)currentChar + 1 <= rasterData->size()) {
                offset = ((row + this->mDescriptor.Top) * lsd->Width) + (col + this->mDescriptor.Left);
                pixelMap->at(offset) = rasterData->at(currentChar);
                currentChar++;
            }
        }
    }
}

void Image::RestoreCanvasToBG(std::vector<char>* pixelMap, LogicalScreenDescriptor* lsd)
{
    logger.Log(TRACE, "Restore canvas to background");
    std::unordered_map<int, std::string> codeTable = LZW::InitializeCodeTable(this->mColorTableSize);

    int offset = 0;
    for (int row = 0; row < this->mDescriptor.Height; row++ ) {
        for (int col = 0; col < this->mDescriptor.Left; col++) {
            offset = ((row + this->mDescriptor.Top) * lsd->Width) + (col + this->mDescriptor.Left);
            pixelMap->at(offset) = codeTable[lsd->BackgroundColorIndex][0]; 
        }
    } 
}

void Image::RestoreToPrevState(std::vector<char>* pixMap, std::vector<char>* prevPixMap)
{
    logger.Log(DEBUG, "Restore canvas to previous state");
    *pixMap = *prevPixMap;
}

void Image::PrintDescriptor()
{
    logger.Log(DEBUG, "------- Image Descriptor -------");
    logger.Log(DEBUG, "Seperator: %X", this->mDescriptor.Seperator);
    logger.Log(DEBUG, "Image Left: %d", this->mDescriptor.Left);
    logger.Log(DEBUG, "Image Top: %d", this->mDescriptor.Top);
    logger.Log(DEBUG, "Image Width: %d", this->mDescriptor.Width);
    logger.Log(DEBUG, "Image Height: %d", this->mDescriptor.Height);
    logger.Log(DEBUG, "Local Color Table Flag: %d", (this->mDescriptor.Packed >> (uint8_t)ImgDescMask::LocalColorTable) & 0x1);
    logger.Log(DEBUG, "Interlace Flag: %d", (this->mDescriptor.Packed >> (uint8_t)ImgDescMask::Interlace) & 0x1);
    logger.Log(DEBUG, "Sort Flag: %d", (this->mDescriptor.Packed >> (uint8_t)ImgDescMask::IMGSort) & 0x1);
    logger.Log(DEBUG, "Size of Local Color Table: %d", (this->mDescriptor.Packed >> (uint8_t)ImgDescMask::IMGSize) & 0x7);
    logger.Log(DEBUG,"--------------------------------");
}

void Image::PrintData()
{
    logger.Log(DEBUG, "------- Image Data -------");
    logger.Log(DEBUG, "LZW Minimum: 0x%X", this->mHeader.LZWMinimum);
    logger.Log(DEBUG, "Initial Follow Size: 0x%X", this->mHeader.FollowSize);
    logger.Log(DEBUG, "--------------------------");
}

void Image::PrintSubBlockData(std::vector<uint8_t>* block)
{
    logger.Log(DEBUG, "------- Block Data -------");
    logger.Log(DEBUG, "Size: %ld\n", block->size());
    for (int i = 0; i < (int)block->size(); i++) {
        fprintf(stdout, "%X ", block->at(i));
    }
    logger.Log(DEBUG, "--------------------------");
}
