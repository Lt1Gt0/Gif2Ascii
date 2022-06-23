#include "gif.h"
#include "colortounicode.h"

#include <stdio.h>
#include <math.h>

GIF::GIF(FILE* fp)
{
    this->file = fp;

    // Get the file size and restore the file pointer back to position 0
    fseek(file, 0, SEEK_END);
    this->filesize = ftell(file);
    rewind(file);
    printf("Total File Size: %.2ldkB\n", (filesize / 1024));

    // Load the GIF header into memory
    this->header = (GifHeader*)malloc(sizeof(GifHeader));
    fread(header, 1, sizeof(GifHeader), file);

    // Check for a valid GIF Header
    if (!ValidHeader()) {
        fprintf(stderr, "Invalid GIF Format header...\n");
        exit(-1);
    } else {
        printf("Valid GIF Header format\n");
    }
}

void GIF::DataLoop()
{
    uint8_t* nextByte = (uint8_t*)malloc(sizeof(uint8_t));
    
    // Initialize the Pixel Map

    // Really stupid way to do this but eventually I will work out the issues
    std::vector<char> pixelMap;
    pixelMap.resize(lsd->Width * lsd->Height);

    // Dont look at this
    // std::vector<std::vector<char>> frameMap;

    // Because the pixel map is not initalized as a multi-dimensional array
    // The way that values of rows and columns will be accessed is like this
    // (char) pixel = PixelMap.at(Row * Width) + Col

    printf("Image Width/Height: %d/%d\n", lsd->Width, lsd->Height);
    for (int row = 0; row < lsd->Height; row++) {
        for (int col = 0; col < lsd->Width; col++) {
            // By default just set the value at the char to ' '
            pixelMap.at((row * lsd->Width) + col) = ' ';
            // printf("%c", PixelMap.at((row * lsd->Width) + col));
        }
        // printf("\n");
    }

    // This while true should build up enough information for the frames of the gif
    while (true) {
        Image img = Image(file, colorTable);

        // Load Image Extenstion information before proceeding with parsing image data
        img.CheckExtensions();
        
        // Load the decompressed image data and draw the frame
        printf("\nLoading Image Data...\n");
        std::string rasterData = img.LoadImageData();

        // printf("Displaying Raster Data...\n");
        // for (char c : rasterData) {
        //     printf("%c", c);
        // }
        // printf("\n");

        img.UpdateFrame(&rasterData, &pixelMap);
        
        fread(nextByte, 1, 1, file);
        fseek(file, -1, SEEK_CUR);
        
        // Check if the file ended correctly (should end on 0x3B)
        if ((size_t)ftell(file) == filesize - 1) {
            if (*nextByte == TRAILER) {
                printf("File Ended Naturally\n");
                break;
            } else {
                printf("File ended unaturally with byte [%X]\n", *nextByte);
                break;
            }
        }
    }
}

void GIF::ReadFileDataHeaders()
{
    lsd = (LogicalScreenDescriptor*)malloc(sizeof(LogicalScreenDescriptor));
    fread(lsd, 1, sizeof(LogicalScreenDescriptor), file);

    // Check to see if the GCT flag is set
    if (lsd->Packed >> LSDMask::GlobalColorTable) {
        printf("Global Color Table Present\nLoading GCT Descriptor...\n");

        colorTable = (std::vector<std::vector<uint8_t>>*)malloc(sizeof(std::vector<std::vector<uint8_t>>));

        // Load the Global Color Table Descriptor Data
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

            colorTable->push_back(color);
            color.clear();
            color.resize(3);
        }
        printf("Successfully Loaded GCT Descriptor\n");

        // Just for a sanity check print out each color in the GCT
        printf("\n------- Global Color Table -------\n");
        for (std::vector<uint8_t> c : *colorTable) {
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