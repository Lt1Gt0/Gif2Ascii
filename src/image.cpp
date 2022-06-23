#include "image.h"

#include <stdio.h>
#include <unordered_map>
#include "lzw.h"
#include "colortounicode.h"

Image::Image(FILE* fp, std::vector<std::vector<uint8_t>>* colortable)
{
    this->file = fp;
    this->colorTable = colortable;
}

void Image::ReadDataSubBlocks(FILE* file)
{
    data.resize(header->FollowSize);

    // Read the first sub block
    for (int i = 0; i < (int)data.size(); i++) {
        fread(&data[i], 1, 1, file);
    }
    
    uint8_t* nextByte = (uint8_t*)malloc(sizeof(uint8_t));
    int dataSize;
    fread(nextByte, 1, 1, file);

    while (true) {
        // Check for the end of sub block
        if (*nextByte == 0x00)
            break;
        
        // Resize the vector to account for the new block to be added
        dataSize = (int)data.size();
        data.resize(dataSize + (int)*nextByte);

        // Read the next (*nextByte) bytes into the vector
        for (int i = 0; i < *nextByte; i++) {
            fread(&data[i + dataSize], 1, 1, file);
        }
        
        fread(nextByte, 1, 1, file);
    }
    
    // for (uint8_t d : data) {
    //     printf("%X ", d);
    // }
    // printf("\n");

    free(nextByte);
}

void Image::LoadExtension(ExtensionHeader* header)
{
    // If the header has a valid label for an extension, start each one
    // by loading the struct into memory and set its header to the one that was passed in
    switch (header->Label) {
    case ExtensionTypes::PlainText:
    {
        printf("Loding Plain Text Extension\n");

        // Load Header
        fseek(file, -2, SEEK_CUR);
        extensions->PlainText = new PlainTextExtension;
        fread(&extensions->PlainText, 1, 2, file);

        // Load the block size into the struct and load the data of that size into the data buffer
        fread(&extensions->PlainText->BlockSize, 1, 1, file);

        extensions->PlainText->Data = (uint8_t*)malloc(sizeof(uint8_t));
        fread(extensions->PlainText->Data, 1, extensions->PlainText->BlockSize, file);
    } break;
    case ExtensionTypes::GraphicsControl:
    {
        printf("Loading Graphics Control Extension\n");

        // Load The entire Graphic Control Extension
        fseek(file, -2, SEEK_CUR);
        extensions->GraphicsControl = new GraphicsControlExtension;
        fread(extensions->GraphicsControl, 1, sizeof(GraphicsControlExtension), file);
    } break;
    case ExtensionTypes::Comment:
    {
        printf("Loding Comment Extension\n");

        // Load Header
        fseek(file, -2, SEEK_CUR);
        extensions->Comment = new CommentExtension;
        fread(&extensions->Comment, 1, 2, file);

        // Read a store bytes until 0x00 is hit
        uint8_t* nextByte = (uint8_t*)malloc(sizeof(uint8_t));
        for (int i = 0; *nextByte != 0x00; i++) {
            fread(nextByte, 1, 1, file);
            // ce->Data[i] = *nextByte;
        }
    } break;
    case ExtensionTypes::Application:
    {
        printf("Loading Application Extension\n");

        // Load Header
        fseek(file, -2, SEEK_CUR);
        extensions->Application = new ApplicationExtension;
        fread(&extensions->Application->Header, 1, 2, file);

        // Load the Block Length
        fread(&extensions->Application->BlockLength, 1, 1, file);

        // Load Application Identifier
        fread(&extensions->Application->Identifier, 1, extensions->Application->BlockLength, file);
        
        // Load the authentication code
        uint8_t tmp;
        fread(&tmp, 1, 1, file);
        fread(&extensions->Application->AuthenticationCode, 1, tmp, file);

        // Check if the next byte in the file is the terminator
        fread(&tmp, 1, 1, file);
        if (tmp == 0x00) {
            printf("End of Application Extension\n");
        }
    } break;
    default:
    {
        fprintf(stderr, "Recived Invalid extension type [%X]\n", header->Label);
    } break;
    }
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
        printf("Loading Local color Table\n");
    else 
        printf("No Local Color Table Flag Set\n");

    // Load the image header into memory
    header = new ImageDataHeader;
    fread(header, 1, 2, file); // Only read 2 bytes of file steam for LZW min and Follow Size 

    ReadDataSubBlocks(file);

    // Get the raster data from the image frame by decompressing the data block from the gif
    std::string rasterData = LZW::Decompress(header, colorTable, data);
    return rasterData;
}

void Image::CheckExtensions()
{
    printf("\nChecking for extensions...\n");
    fpos_t prevPos;

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

std::vector<char> Image::UpdateFrame(std::string* rasterData, std::vector<char>* origPixMap, int gifWidth, int gifHeight)
{
    // Initialize a code table so I know what color/code to output
    // std::unordered_map<int, std::string> codeTable = LZW::InitializeCodeTable(colorTable);

    // Because each gif can have a different disposal method for different frames (according to GIF89a)
    // it is best to handle each disposal method instread of printing the decompressed codestream directly
    int disposalMethod = ((extensions->GraphicsControl->Packed >> Disposal) & 0x07);
    printf("Disposal Method: %d\n", disposalMethod);
    switch (disposalMethod) {
    case 0:
        break;
    case 1:
        DrawOverImage(rasterData, origPixMap, gifWidth, gifHeight);
        break;
    case 2:
        RestoreCanvasToBG(rasterData, origPixMap);
        break;
    case 3:
        RestoreToPrevState(rasterData, origPixMap);
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        break;
    default:
        fprintf(stderr, "Undefined Disposal Method: %d\n", disposalMethod);
        exit(-1);
        break;
    }

    // I really need to work on this nomenclature
    return *origPixMap;
}

void Image::PrintDescriptor()
{
    printf("------- Image Descriptor -------\n");
    printf("Seperator: %X\n", descriptor->Seperator);
    printf("Image Left: %d\n", descriptor->Left);
    printf("Image Top: %d\n", descriptor->Top);
    printf("Image Width: %d\n", descriptor->Width);
    printf("Image Height: %d\n", descriptor->Height);
    printf("Local Color Table Flag: %d\n", (descriptor->Packed >> ImgDescMask::LocalColorTable) & 0x1);
    printf("Interlace Flag: %d\n", (descriptor->Packed >> ImgDescMask::Interlace) & 0x1);
    printf("Sort Flag: %d\n", (descriptor->Packed >> ImgDescMask::IMGSort) & 0x1);
    printf("Size of Local Color Table: %d\n", (descriptor->Packed >> ImgDescMask::IMGSize) & 0x7);
    printf("--------------------------------\n");
}

void Image::PrintData()
{
    printf("\n------- Image Data -------\n");
    printf("LZW Minimum: 0x%X\n", header->LZWMinimum);
    printf("Initial Follow Size: 0x%X\n", header->FollowSize);
    printf("--------------------------\n");
}

void Image::PrintSubBlockData(std::vector<uint8_t> block)
{
    printf("\n------- Block Data -------\n");
    printf("Size: %ld\n", block.size());
    for (int i = 0; i < (int)block.size(); i++) {
        printf("%X ", block.at(i));
    }
    printf("\n--------------------------\n");
}

void Image::DrawOverImage(std::string* rasterData, std::vector<char>* pixelMap, int gifWidth, int gifHeight)
{
    printf("Drawing Over Image\n");
    int offset = (descriptor->Top * gifWidth) + descriptor->Left;

    int currentChar = 0;
    for (int row = descriptor->Top; row < descriptor->Height; row++) {
        for (int col = descriptor->Left; col < descriptor->Width; col++) {
            offset = (row * gifWidth) + col;
            pixelMap->at(offset) = rasterData->at(currentChar);
            currentChar++;
        }
    }
}

void Image::RestoreCanvasToBG(std::string* rasterData, std::vector<char>* pixelMap)
{
    printf("Restore Canvas to BackGround\n");
}

void Image::RestoreToPrevState(std::string* rasterData, std::vector<char>* pixelMap)
{
    printf("Restore Canvas to Previous State\n");
}