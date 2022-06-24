#include "image.h"

#include <stdio.h>
#include <unordered_map>
#include "lzw.h"
#include "colortounicode.h"
#include "errorhandler.h"
#include "Debug/debug.h"

Image::Image(FILE* fp, std::vector<std::vector<uint8_t>>* colortable)
{
    this->file = fp;
    this->colorTable = colortable;

    this->descriptor = new ImageDescriptor;
    this->header = new ImageDataHeader;
    this->extensions = new ImageExtensions;
    this->data = std::vector<uint8_t>();
}

std::string Image::LoadImageData()
{
    // Load the Image Descriptor into memory
    descriptor = new ImageDescriptor;
    fread(descriptor, 1, sizeof(ImageDescriptor), file);

    // Load the Local Color Table if it is set 
    // The current GIF I am trying to target does not have a need for a LCT so I am just
    // going to skip loading one for now (sucks to suck)
    if ((descriptor->Packed >> ImgDescMask::LocalColorTable) & 0x1)
        Debug::Print("Loading Local Color Table");
    else
        Debug::Print("No Local Color Table Flag Set");

    // Load the image header into memory
    header = new ImageDataHeader;
    fread(header, 1, sizeof(ImageDataHeader), file); // Only read 2 bytes of file steam for LZW min and Follow Size 
    ReadDataSubBlocks(file);

    // Get the raster data from the image frame by decompressing the data block from the gif
    std::string rasterData = LZW::Decompress(header, colorTable, data);
    return rasterData;
}

void Image::ReadDataSubBlocks(FILE* file)
{
    uint8_t nextByte;
    int followSize;
    followSize = header->FollowSize;
    
    while (followSize--) {
        fread(&nextByte, 1, sizeof(uint8_t), file);
        data.push_back(nextByte);
    }

    fread(&nextByte, 1, sizeof(uint8_t), file);

    while (true) {
        // Check for the end of sub block
        if (!nextByte)
            break;
        
        followSize = nextByte;
        while (followSize--) {
            fread(&nextByte, 1, sizeof(uint8_t), file);
            data.push_back(nextByte);
        }

        fread(&nextByte, 1, sizeof(uint8_t), file);
    }

    // Print out the compressed stream of data 
    for (uint8_t d : data) {
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
        fread(extensionCheck, 1, sizeof(ExtensionHeader), file);

        // If the dummy header contains an introducer for a extension, load the extension type
        if (extensionCheck->Introducer == EXTENSION_INTRODUCER) {
            LoadExtension(extensionCheck);
        } else {
            fseek(file, -2, SEEK_CUR);
            free(extensionCheck);
            return;
        }
    }
}

void Image::LoadExtension(ExtensionHeader* header)
{
    // If the header has a valid label for an extension, start each one
    // by loading the struct into memory and set its header to the one that was passed in
    fseek(file, -2, SEEK_CUR);

    switch (header->Label) {
    case ExtensionTypes::PlainText:
    {
        Debug::Print("Loding Plain Text Extension");

        // Load Header
        extensions->PlainText = new PlainTextExtension;
        fread(&extensions->PlainText->Header, 1, sizeof(ExtensionHeader), file);

        // Load the block size into the struct and load the data of that size into the data buffer
        fread(&extensions->PlainText->BlockSize, 1, sizeof(uint8_t), file);

        extensions->PlainText->Data = (uint8_t*)malloc(sizeof(uint8_t) * extensions->PlainText->BlockSize);
        fread(extensions->PlainText->Data, 1, extensions->PlainText->BlockSize, file);
    } break;
    case ExtensionTypes::GraphicsControl:
    {
        Debug::Print("Loading Graphics Control Extension");

        // Load The entire Graphic Control Extension
        extensions->GraphicsControl = new GraphicsControlExtension;
        fread(extensions->GraphicsControl, 1, sizeof(GraphicsControlExtension), file);
    } break;
    case ExtensionTypes::Comment:
    {
        Debug::Print("Loding Comment Extension");

        // Load Header
        extensions->Comment = new CommentExtension;
        fread(&extensions->Comment->Header, 1, sizeof(ExtensionHeader), file);

        // Read a store bytes until 0x00 is hit
        // Just as a reminder, I am discarding the comment information for now, implement later if needed
        uint8_t nextByte;
        for (int i = 0; nextByte != 0x00; i++) {
            fread(&nextByte, 1, sizeof(uint8_t), file);
        }
    } break;
    case ExtensionTypes::Application:
    {
        Debug::Print("Loading Application Extension");

        // Load Header
        extensions->Application = new ApplicationExtension;
        // extensions->Application = (ApplicationExtension*)malloc(sizeof(ApplicationExtension));
        fread(&extensions->Application->Header, 1, sizeof(ExtensionHeader), file);

        // Load the Block Length
        fread(&extensions->Application->BlockLength, 1, sizeof(uint8_t), file);

        // Load Application Identifier
        fread(&extensions->Application->Identifier, 1, extensions->Application->BlockLength, file);
        
        // Load the authentication code
        uint8_t tmp;
        fread(&tmp, 1, sizeof(uint8_t), file);
        fread(&extensions->Application->AuthenticationCode, 1, tmp, file);

        // Check if the next byte in the file is the terminator
        fread(&tmp, 1, sizeof(uint8_t), file);
        if (!tmp)
            Debug::Print("End of Application Extension");
    } break;
    default:
    {
        Debug::PrintErr("Recived Invalid extension type [%X]", header->Label);
        fseek(file, 2, SEEK_CUR); // Restore the file position to where it was after reading header
    } break;
    }
}

void Image::UpdatePixelMap(std::vector<char>* pixMap, std::string* rasterData, LogicalScreenDescriptor* lsd)
{
    // Because each gif can have a different disposal method for different frames (according to GIF89a)
    // it is best to handle each disposal method instread of printing the decompressed codestream directly
    int disposalMethod = ((extensions->GraphicsControl->Packed >> Disposal) & 0x07);
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
    for (int row = 0; row < descriptor->Height; row++) {
        for (int col = 0; col < descriptor->Width; col++) {
            if (currentChar + 1 <= rasterData->size()) {
                offset = ((row + descriptor->Top) * lsd->Width) + (col + descriptor->Left);
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
    Debug::Print("Seperator: %X", descriptor->Seperator);
    Debug::Print("Image Left: %d", descriptor->Left);
    Debug::Print("Image Top: %d", descriptor->Top);
    Debug::Print("Image Width: %d", descriptor->Width);
    Debug::Print("Image Height: %d", descriptor->Height);
    Debug::Print("Local Color Table Flag: %d", (descriptor->Packed >> ImgDescMask::LocalColorTable) & 0x1);
    Debug::Print("Interlace Flag: %d", (descriptor->Packed >> ImgDescMask::Interlace) & 0x1);
    Debug::Print("Sort Flag: %d", (descriptor->Packed >> ImgDescMask::IMGSort) & 0x1);
    Debug::Print("Size of Local Color Table: %d", (descriptor->Packed >> ImgDescMask::IMGSize) & 0x7);
    Debug::Print("--------------------------------");
}

void Image::PrintData()
{
    Debug::Print("\n------- Image Data -------");
    Debug::Print("LZW Minimum: 0x%X", header->LZWMinimum);
    Debug::Print("Initial Follow Size: 0x%X", header->FollowSize);
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