#include "image.h"

#include <stdio.h>
#include <unordered_map>
#include "lzw.h"
#include "colortounicode.h"
#include "errorhandler.h"
#include "Debug/debug.h"

Image::Image(FILE* _fp, Color* _colortable, uint8_t _colorTableSize)
{
    this->mFile = _fp;
    this->mColorTable = _colortable;
    this->mColorTableSize = _colorTableSize;

    this->mDescriptor = new ImageDescriptor;
    this->mHeader = new ImageDataHeader;
    this->mExtensions = new ImageExtensions;
    this->mData = std::vector<uint8_t>();
}

std::string Image::LoadImageData()
{
    // Load the Image Descriptor into memory
    this->mDescriptor = new ImageDescriptor;
    fread(this->mDescriptor, 1, sizeof(ImageDescriptor), this->mFile);

    // Load the Local Color Table if it is set 
    // The current GIF I am trying to target does not have a need for a LCT so I am just
    // going to skip loading one for now (sucks to suck)
    if ((this->mDescriptor->Packed >> ImgDescMask::LocalColorTable) & 0x1)
        Debug::Print("Loading Local Color Table");
    else
        Debug::Print("No Local Color Table Flag Set");

    // Load the image header into memory
    this->mHeader = new ImageDataHeader;
    fread(this->mHeader, 1, sizeof(ImageDataHeader), this->mFile); // Only read 2 bytes of file steam for LZW min and Follow Size 
    ReadDataSubBlocks();

    // Get the raster data from the image frame by decompressing the data block from the gif
    std::string rasterData = LZW::Decompress(this->mHeader, this->mColorTableSize, this->mData);
    return rasterData;
}

void Image::ReadDataSubBlocks()
{
    uint8_t nextByte;
    int followSize;
    followSize = this->mHeader->FollowSize;
    
    while (followSize--) {
        fread(&nextByte, 1, sizeof(uint8_t), this->mFile);
        this->mData.push_back(nextByte);
    }

    fread(&nextByte, 1, sizeof(uint8_t), this->mFile);

    while (true) {
        // Check for the end of sub block
        if (!nextByte)
            break;
        
        followSize = nextByte;
        while (followSize--) {
            fread(&nextByte, 1, sizeof(uint8_t), this->mFile);
            this->mData.push_back(nextByte);
        }

        fread(&nextByte, 1, sizeof(uint8_t), this->mFile);
    }

    // Print out the compressed stream of data 
    for (uint8_t d : this->mData) {
        fprintf(stdout, "%X ", d);
    }
    
    fprintf(stdout, "\n");
    fflush(stdout);
}

void Image::CheckExtensions()
{
    Debug::Print("\nChecking for extensions...");

    // Load a dummy header into memory
    ExtensionHeader* extensionCheck = (ExtensionHeader*)malloc(sizeof(ExtensionHeader));

    while (true) {
        fread(extensionCheck, 1, sizeof(ExtensionHeader), this->mFile);

        // If the dummy header contains an introducer for a extension, load the extension type
        if (extensionCheck->Introducer == EXTENSION_INTRODUCER) {
            LoadExtension(extensionCheck);
        } else {
            fseek(this->mFile, -2, SEEK_CUR);
            free(extensionCheck);
            return;
        }
    }
}

void Image::LoadExtension(ExtensionHeader* header)
{
    // If the header has a valid label for an extension, start each one
    // by loading the struct into memory and set its header to the one that was passed in
    fseek(this->mFile, -2, SEEK_CUR);

    switch (header->Label) {
    case ExtensionTypes::PlainText:
    {
        Debug::Print("Loding Plain Text Extension");

        // Load Header
        mExtensions->PlainText = new PlainTextExtension;
        fread(&mExtensions->PlainText->Header, 1, sizeof(ExtensionHeader), this->mFile);

        // Load the block size into the struct and load the data of that size into the data buffer
        fread(&mExtensions->PlainText->BlockSize, 1, sizeof(uint8_t), this->mFile);

        mExtensions->PlainText->Data = (uint8_t*)malloc(sizeof(uint8_t) * mExtensions->PlainText->BlockSize);
        fread(mExtensions->PlainText->Data, 1, mExtensions->PlainText->BlockSize, this->mFile);
    } break;
    case ExtensionTypes::GraphicsControl:
    {
        Debug::Print("Loading Graphics Control Extension");

        // Load The entire Graphic Control Extension
        mExtensions->GraphicsControl = new GraphicsControlExtension;
        fread(mExtensions->GraphicsControl, 1, sizeof(GraphicsControlExtension), this->mFile);
    } break;
    case ExtensionTypes::Comment:
    {
        Debug::Print("Loding Comment Extension");

        // Load Header
        mExtensions->Comment = new CommentExtension;
        fread(&mExtensions->Comment->Header, 1, sizeof(ExtensionHeader), this->mFile);

        // Read a store bytes until 0x00 is hit
        // Just as a reminder, I am discarding the comment information for now, implement later if needed
        uint8_t nextByte = 0;
        for (int i = 0; nextByte != 0x00; i++) {
            fread(&nextByte, 1, sizeof(uint8_t), this->mFile);
        }
    } break;
    case ExtensionTypes::Application:
    {
        Debug::Print("Loading Application Extension");

        // Load Header
        mExtensions->Application = new ApplicationExtension;
        // extensions->Application = (ApplicationExtension*)malloc(sizeof(ApplicationExtension));
        fread(&mExtensions->Application->Header, 1, sizeof(ExtensionHeader), this->mFile);

        // Load the Block Length
        fread(&mExtensions->Application->BlockLength, 1, sizeof(uint8_t), this->mFile);

        // Load Application Identifier
        fread(&mExtensions->Application->Identifier, 1, mExtensions->Application->BlockLength, this->mFile);
        
        // Load the authentication code
        uint8_t tmp;
        fread(&tmp, 1, sizeof(uint8_t), this->mFile);
        fread(&mExtensions->Application->AuthenticationCode, 1, tmp, this->mFile);

        // Check if the next byte in the file is the terminator
        fread(&tmp, 1, sizeof(uint8_t), this->mFile);
        if (!tmp)
            Debug::Print("End of Application Extension");
    } break;
    default:
    {
        Debug::PrintErr("Recived Invalid extension type [%X]", header->Label);
        fseek(this->mFile, 2, SEEK_CUR); // Restore the file position to where it was after reading header
    } break;
    }
}

void Image::UpdatePixelMap(std::vector<char>* pixMap, std::string* rasterData, LogicalScreenDescriptor* lsd)
{
    // Because each gif can have a different disposal method for different frames (according to GIF89a)
    // it is best to handle each disposal method instread of printing the decompressed codestream directly
    int disposalMethod = ((mExtensions->GraphicsControl->Packed >> Disposal) & 0x07);
    switch (disposalMethod) {
    case 0:
        break;
    case 1:
        DrawOverImage(rasterData, pixMap, lsd);
        break;
    case 2:
        RestoreCanvasToBG(rasterData, pixMap);
        break;
    case 3:
        RestoreToPrevState(rasterData, pixMap);
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        break;
    default:
        ErrorHandler::err_n_die("Undefined Disposal Method: %d\n", Disposal);
        break;
    }
}

void Image::DrawOverImage(std::string* rasterData, std::vector<char>* pixelMap, LogicalScreenDescriptor* lsd)
{
    Debug::Print("Drawing Over Image");
    int offset;
    int currentChar = 0;
    for (int row = 0; row < this->mDescriptor->Height; row++) {
        for (int col = 0; col < this->mDescriptor->Width; col++) {
            if ((size_t)currentChar + 1 <= rasterData->size()) {
                offset = ((row + this->mDescriptor->Top) * lsd->Width) + (col + this->mDescriptor->Left);
                pixelMap->at(offset) = rasterData->at(currentChar);
                currentChar++;
            }
        }
    }
}

void Image::RestoreCanvasToBG(std::string* rasterData, std::vector<char>* pixelMap)
{
    Debug::Print("Restore Canvas to Background");
}

void Image::RestoreToPrevState(std::string* rasterData, std::vector<char>* pixelMap)
{
    Debug::Print("Restore Canvas to Previous State");
}

void Image::PrintDescriptor()
{
    Debug::Print("------- Image Descriptor -------");
    Debug::Print("Seperator: %X", this->mDescriptor->Seperator);
    Debug::Print("Image Left: %d", this->mDescriptor->Left);
    Debug::Print("Image Top: %d", this->mDescriptor->Top);
    Debug::Print("Image Width: %d", this->mDescriptor->Width);
    Debug::Print("Image Height: %d", this->mDescriptor->Height);
    Debug::Print("Local Color Table Flag: %d", (this->mDescriptor->Packed >> ImgDescMask::LocalColorTable) & 0x1);
    Debug::Print("Interlace Flag: %d", (this->mDescriptor->Packed >> ImgDescMask::Interlace) & 0x1);
    Debug::Print("Sort Flag: %d", (this->mDescriptor->Packed >> ImgDescMask::IMGSort) & 0x1);
    Debug::Print("Size of Local Color Table: %d", (this->mDescriptor->Packed >> ImgDescMask::IMGSize) & 0x7);
    Debug::Print("--------------------------------");
}

void Image::PrintData()
{
    Debug::Print("\n------- Image Data -------");
    Debug::Print("LZW Minimum: 0x%X", this->mHeader->LZWMinimum);
    Debug::Print("Initial Follow Size: 0x%X", this->mHeader->FollowSize);
    Debug::Print("--------------------------");
}

void Image::PrintSubBlockData(std::vector<uint8_t>* block)
{
    Debug::Print("\n------- Block Data -------");
    Debug::Print("Size: %ld\n", block->size());
    for (int i = 0; i < (int)block->size(); i++) {
        fprintf(stdout, "%X ", block->at(i));
    }
    Debug::Print("\n--------------------------");
}
