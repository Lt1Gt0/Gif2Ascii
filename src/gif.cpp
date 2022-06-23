#include "gif.h"

#include <iostream>
#include <tgmath.h>

GIF::GIF(FILE* fp)
{
    file = fp;

    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
    rewind(file);
    printf("Total File Size: %.2ldkB\n", (filesize / 1024));
}

int GIF::LoadHeader()
{
    header = (Header*)malloc(sizeof(Header));
    fread(header, 1, sizeof(Header), file);

    return ValidHeader();
}

void GIF::DataLoop()
{
    fpos_t prevPos;
    uint8_t* nextByte = (uint8_t*)malloc(sizeof(uint8_t));
    fgetpos(file, &prevPos);
    fread(nextByte, 1, 1, file);
    // fsetpos(file, &prevPos);
    while (*nextByte != TRAILER) {
        // printf("Next Byte in file before rewinding: %X\n", *nextByte);
        // fsetpos(file, &prevPos);
        CheckExtensions();
        fgetpos(file, &prevPos);
        fread(nextByte, 1, 1, file);
    
        // I dont know why I do it like this but just use this
        // to make sure I hit the Imaga data and then read it
        // (also the loop just removes whatever isn't 0x2C so 
        // if fo some reason my code acts up at around this spot
        // this might be the culprit)
        while (*nextByte != 0x2C) {
            fgetpos(file, &prevPos);
            fread(nextByte, 1, 1, file);
        }

        fsetpos(file, &prevPos);
        printf("\nLoading Image Data...\n");
        LoadImageData();
        
        if (ftell(file) != EOF) {
            fread(nextByte, 1, 1, file);
        }
    }

    printf("Finished Reading GIF\n");
}

void GIF::ReadFileDataHeaders()
{
    lsd = (LogicalScreenDescriptor*)malloc(sizeof(LogicalScreenDescriptor));
    fread(lsd, 1, sizeof(LogicalScreenDescriptor), file);

    // Check to see if the GCT flag is set
    if (lsd->Packed >> LSDMask::GlobalColorTable) {
        printf("Global Color Table Present\nLoading GCT Descriptor...\n");
        gctd = (GlobalColorTableDescriptor*)malloc(sizeof(GlobalColorTableDescriptor));
        gctd->SizeInLSD = (lsd->Packed >> LSDMask::LSDSize) & 0x07;
        gctd->NumberOfColors = pow(2, gctd->SizeInLSD + 1);
        gctd->ByteLegth = 3 * gctd->NumberOfColors;
        gct = malloc(gctd->ByteLegth);
        fread(gct, 1, gctd->ByteLegth, file);
        printf("Successfully Loaded GCT Descriptor\n");
    } else {
        printf("Global Color Table Not Present\n");
    }
}

bool GIF::ValidHeader()
{
    for (int i = 0; i < 6; i++) {
        if (header->Signature[i] != GIF_MAGIC_0[i]
         && header->Signature[i] != GIF_MAGIC_1[i]) {
            return false;
        } 
    }

    return true;
}

void GIF::LoadExtension(ExtensionHeader* header)
{
    using namespace GIF_Headers;
    
    switch (header->Label) {
    case ExtensionTypes::PlainText:
    {
        printf("Loding Plain Text Extension\n");
        PlainTextExtension* pe = (PlainTextExtension*)malloc(sizeof(PlainTextExtension));
        pe->header = *header;
        fread(&pe->BlockSize, 1, 1, file);
        printf("Block Size: %d\n", pe->BlockSize);
        uint8_t* dataBuf = (uint8_t*)malloc(sizeof(uint8_t) * pe->BlockSize);
        fread(dataBuf, 1, pe->BlockSize, file);

    } break;
    case ExtensionTypes::GraphicsControl:
    {
        printf("Loading Graphics Control Extension\n");
        GraphicsControlExtension* gce = (GraphicsControlExtension*)malloc(sizeof(GraphicsControlExtension));
        gce->header = *header;
        fread(gce, 1, sizeof(GraphicsControlExtension) - sizeof(ExtensionHeader) + 1, file);
        free(gce); // Discarded Graphic Control Extension
        printf("Loaded Graphics Control Extension\n");
    } break;
    case ExtensionTypes::Comment:
    {
        printf("Loding Comment Extension\n");
        CommentExtension* ce = (CommentExtension*)malloc(sizeof(CommentExtension));
        ce->header = *header;

        uint8_t* nextByte = (uint8_t*)malloc(sizeof(uint8_t));

        while (*nextByte != 0x00) {
            fread(nextByte, 1, 1, file);
            // Notice how I do nothing with the data other
            // than to read it so I can advance the file pointer??

            // TODO: Make the comment extension data sub blocks actuatually
            // store the value in the file (if I feel like it though)
        }
    } break;
    case ExtensionTypes::Application:
    {
        printf("Loding Application Extension\n");
        /* I AM DISCARDING APPLICATION EXTENSIONS FOR THE TIME BEING CAUSE I DON'T NEED THEM */

        ApplicationExtension* ae = (ApplicationExtension*)malloc(sizeof(ApplicationExtension));
        ae->header = *header;
        fread(&ae->BlockLength, 1, 1, file);
        printf("Application Block Length: %d\n", ae->BlockLength);

        ae->Identifier = (uint8_t*)malloc(sizeof(uint8_t) * ae->BlockLength);
        fread(&ae->Identifier, 1, ae->BlockLength, file);
        // ae->Identifier = identBuf;

        uint8_t authLength;
        fread(&authLength, 1, 1, file);

        ae->AuthenticationCode = (uint8_t*)malloc(sizeof(uint8_t) * authLength);
        fread(ae->AuthenticationCode, 1, authLength, file);
        // ae->AuthenticationCode = authBuf;

        uint8_t* next = (uint8_t*)malloc(sizeof(uint8_t));
        fread(next, 1, 1, file);
        if (*next == 0x00) {
            printf("Application block end\n");
        }
        // Read terminator value and mov file position forward
        // fread(&ae->Terminator, 1, 1, file);
        
        // free(ae); // Discarded Application extension
    } break;
    default:
    {
        fprintf(stderr, "Recived Invalid extension type [%X]\n", header->Label);
    } break;
    }
}

void GIF::LoadImageData()
{
    // Load the Image Descriptor
    ImageDescriptor* imageDescriptor = (ImageDescriptor*)malloc(sizeof(ImageDescriptor));
    fread(imageDescriptor, 1, sizeof(ImageDescriptor), file);
    // PrintImageDescriptor(imageDescriptor);

    // Load the Local Color Table if it is set
    if ((imageDescriptor->Packed >> ImgDescMask::LocalColorTable) & 0x1) {
        printf("Loading Local color Table\n");
        // The current GIF I am trying to target does not have a need for a LCT so I am just
        // going to skip loading one for now (sucks to suck)
    } else {
        printf("No Local Color Table Flag Set");
    }

    //Load Image Data (Time to implement LZW Compression)
    ImageData* imageData = (ImageData*)malloc(sizeof(ImageData));
    uint8_t* subBlockData;
    fread(imageData, 1, 2, file); // Only read 2 bytes of file steam for LZW min and Follow Size
    PrintImageData(imageData);
    subBlockData = ReadImgDataSubBlock(imageData);

    printf("Current File Pos: 0x%lX\n", ftell(file));
}

uint8_t* GIF::ReadImgDataSubBlock(ImageData* imgData)
{
    size_t subBlockSize = imgData->FollowSize;
    uint8_t* subBlock = (uint8_t*)malloc(sizeof(uint8_t) * subBlockSize);
    fread(subBlock, 1, subBlockSize, file);

    uint8_t* nextByte = (uint8_t*)malloc(sizeof(uint8_t));
    bool readingSubBlocks = true;

    while (readingSubBlocks) {
        fread(nextByte, 1, 1, file);
        printf("Next Follow Size: 0x%X\n", *nextByte);

        if (*nextByte != 0x00) {
            subBlockSize += *nextByte;
            subBlock = (uint8_t*)realloc(subBlock, subBlockSize);
            fread(subBlock, 1, *nextByte, file);
        } else {
            printf("Finished Reading Data Sub Block\n");
            readingSubBlocks = false;
        }
    }

    return subBlock;
}

void GIF::CheckExtensions()
{
    printf("\nChecking for extensions...\n");
    fpos_t prevPos;
    ExtensionHeader* extensionCheck = (ExtensionHeader*)malloc(sizeof(ExtensionHeader));
    bool loadingExtensions = true;
    while (loadingExtensions) {
        fgetpos(file, &prevPos);
        fread(extensionCheck, 1, sizeof(ExtensionHeader), file);
    
        if (extensionCheck->Introducer == EXTENSION_INTRODUCER) {
            LoadExtension(extensionCheck);
        } else {
            fsetpos(file, &prevPos);
            free(extensionCheck);
            loadingExtensions = false;
        }
    }
}

void GIF::PrintHeaderInfo()
{   
    printf("\n------- GIF INFO -------\n");

    printf("[Header]\n");
    printf("\tSignature: %s\n", header->Signature);
    printf("\tVersion: %s\n", header->Version);

    printf("[Logical Screen Descriptor]\n");
    printf("\tWidth: %d\n", lsd->Width);
    printf("\tHeight: %d\n", lsd->Height);
    printf("\tGlobal Color Table Flag: %d\n", (lsd->Packed >> LSDMask::GlobalColorTable) & 0x1);
    printf("\tColor Resolution: %d\n", (lsd->Packed >> LSDMask::ColorResolution) & 0x07);
    printf("\tSort Flag: %d\n", (lsd->Packed >> LSDMask::LSDSort) & 0x01);
    printf("\tGlobal Color Table Size: %d\n", (lsd->Packed >> LSDMask::LSDSize) & 0x07);
    printf("\tBackground Color Index: %d\n", lsd->BackgroundColorIndex);
    printf("\tPixel Aspect Ratio: %d\n", lsd->PixelAspectRatio);

    if (lsd->Packed >> LSDMask::GlobalColorTable) {
        printf("[Global Color Table]\n");
        printf("\tSize: %d\n", gctd->SizeInLSD);
        printf("\tNumber of Colors: %d\n", gctd->NumberOfColors);
        printf("\tSize in bytes: %d\n", gctd->ByteLegth);
        // printf("\tLocated in memory address: %p\n", gct);
    }
    
    // printf("[Graphic Control Extension]\n");
    // printf("\tExtension Introducer: %X\n", gce->header.Introducer);
    // printf("\tGraphic Control Label: %X\n", gce->header.Label);
    // printf("\tDisposal Method: %d\n", (gce->Packed >> GCEMask::Disposal) & 0x07);
    // printf("\tUser Input Flag: %d\n", (gce->Packed >> GCEMask::UserInput) & 0x01);
    // printf("\tTransparent Flag: %d\n", (gce->Packed >> GCEMask::TransparentColor) & 0x01);
    // printf("\tDelay Time: %d\n", gce->DelayTime);
    // printf("\tTransparent Color Index: %d\n", gce->TransparentColorIndex);
    // printf("\tBlock Terminator: %d\n", gce->BlockTerminator);
    // printf("------------------------\n");
}

void GIF::PrintImageDescriptor(ImageDescriptor* descriptor)
{
    printf("------- Image Descriptor -------\n");
    printf("Seperator: %X\n", descriptor->Seperator);
    printf("Image Left: %d\n", descriptor->Left);
    printf("Image Right: %d\n", descriptor->Right);
    printf("Image Width: %d\n", descriptor->Width);
    printf("Image Height: %d\n", descriptor->Height);
    printf("Local Color Table Flag: %d\n", (descriptor->Packed >> ImgDescMask::LocalColorTable) & 0x1);
    printf("Interlace Flag: %d\n", (descriptor->Packed >> ImgDescMask::Interlace) & 0x1);
    printf("Sort Flag: %d\n", (descriptor->Packed >> ImgDescMask::IMGSort) & 0x1);
    printf("Size of Local Color Table: %d\n", (descriptor->Packed >> ImgDescMask::IMGSize) & 0x7);
    printf("--------------------------------\n");
}

void GIF::PrintImageData(ImageData* data)
{
    printf("\n------- Image Data -------\n");
    printf("LZW Minimum: 0x%X\n", data->LZWMinimum);
    printf("Initial Follow Size: 0x%X\n", data->FollowSize);
    printf("--------------------------\n");
}