#include "gif.h"

#include <iostream>
#include <math.h>

GIF::GIF(FILE* fp)
{
    file = fp;

    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
    rewind(file);
    printf("Total File Size: %.2ldkB\n", (filesize / 1024));

    header = (Header*)malloc(sizeof(Header));
    fread(header, 1, sizeof(Header), file);
    if (!ValidHeader()) {
        fprintf(stderr, "Invalid GIF Format header...\n");
        exit(-1);
    } else {
        printf("Valid GIF Header format");
    }
}

void GIF::DataLoop()
{
    fpos_t prevPos;
    uint8_t* nextByte = (uint8_t*)malloc(sizeof(uint8_t));
    fgetpos(file, &prevPos);
    fread(nextByte, 1, 1, file);
    bool loop = true;
    while (loop) {
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
 
        fread(nextByte, 1, 1, file);
        if ((size_t)ftell(file) == filesize) {
            if (*nextByte == TRAILER) {
                printf("File Ended Naturally\n");
                loop = false;
            } else {
                printf("File ended unaturally with byte [%X]\n", *nextByte);
                loop = false;
            }
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

        // Generate the GCT from each color present in file
        std::vector<uint8_t> color;
        color.resize(3);
        for (int i = 0; i < gctd->NumberOfColors; i++) {
            for (int i = 0; i < (int)color.size(); i++) {
                fread(&color[i], 1, 1, file);
            }

            colorTable.push_back(color);
            color.clear();
            color.resize(3);
        }
        printf("Successfully Loaded GCT Descriptor\n");

        printf("\n------- Global Color Table -------\n");
        // Just for a sanity check
        for (std::vector<uint8_t> c : colorTable) {
            printf("Red: %X\n", c[0]);
            printf("Green: %X\n", c[1]);
            printf("Blue: %X\n\n", c[2]);
        }
        printf("----------------------------------\n");
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
    } break;
    default:
    {
        fprintf(stderr, "Recived Invalid extension type [%X]\n", header->Label);
    } break;
    }
}

void GIF::LoadImageData()
{
    using namespace ImageData;

    // Load the Image Descriptor
    Image img = Image();
    img.descriptor = (ImageDescriptor*)malloc(sizeof(ImageDescriptor));
    fread(img.descriptor, 1, sizeof(ImageDescriptor), file);

    // Load the Local Color Table if it is set
    if ((img.descriptor->Packed >> ImgDescMask::LocalColorTable) & 0x1) {
        printf("Loading Local color Table\n");
        // The current GIF I am trying to target does not have a need for a LCT so I am just
        // going to skip loading one for now (sucks to suck)
    } else {
        printf("No Local Color Table Flag Set\n");
    }

    //Load Image Data (Time to implement LZW Compression) (Update: THIS SUCKS WHY DID I WANT TO DO THIS)
    img.header = (ImageDataHeader*)malloc(sizeof(ImageDataHeader));
    fread(img.header, 1, 2, file); // Only read 2 bytes of file steam for LZW min and Follow Size 

    img.PrintData();
    img.ReadDataSubBlocks(file);
    imageData.push_back(img);


    printf("Block count: %d\n", (int)img.subBlocks.size());
    for (std::vector<uint8_t> block : img.subBlocks) {
        LZW::Decompress(&img, colorTable, block);
    }

    printf("Current File Pos: 0x%lX\n", ftell(file));
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
    }
}