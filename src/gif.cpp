#include "gif.h"
#include "colortounicode.h"
#include "lzw.h"

#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <unordered_map>

GIF::GIF(FILE* fp)
{
    this->file = fp;

    // Get the file size and restore the file pointer back to position 0
    fseek(file, 0, SEEK_END);
    this->filesize = ftell(file);
    rewind(file);
    fprintf(stdout, "Total File Size: %.2ldkB\n", (filesize / 1024));

    // Load the GIF header into memory
    this->header = new GifHeader; 
    fread(header, 1, sizeof(GifHeader), file);

    // Check for a valid GIF Header
    if (!ValidHeader()) {
        fprintf(stderr, "Invalid GIF Format header...\n");
        exit(-1);
    } else {
        fprintf(stdout, "Valid GIF Header format\n");
    }

    this->lsd = new LogicalScreenDescriptor;
    this->imageData = std::vector<Image>();
    this->frameMap = std::vector<std::vector<char>>();
}

void GIF::ReadFileDataHeaders()
{
    fread(lsd, 1, sizeof(LogicalScreenDescriptor), file);

    // Check to see if the GCT flag is set
    if (lsd->Packed >> LSDMask::GlobalColorTable) {
        fprintf(stdout, "Global Color Table Present\nLoading GCT Descriptor...\n");

        colorTable = (std::vector<std::vector<uint8_t>>*)malloc(sizeof(std::vector<std::vector<uint8_t>>));

        // Load the Global Color Table Descriptor Data
        gctd = new GlobalColorTableDescriptor;
        gctd->SizeInLSD = (lsd->Packed >> LSDMask::LSDSize) & 0x07;
        gctd->NumberOfColors = pow(2, gctd->SizeInLSD + 1);
        gctd->ByteLegth = 3 * gctd->NumberOfColors;

        // Generate the GCT from each color present in file
        std::vector<uint8_t> color;
        color.resize(3);
        for (int i = 0; i < gctd->NumberOfColors; i++) {
            for (int i = 0; i < (int)color.size(); i++) {
                fread(&color[i], 1, sizeof(char), file);
            }

            colorTable->push_back(color);
            color.clear();
            color.resize(3);
        }
        fprintf(stdout, "Successfully Loaded GCT Descriptor\n");

        // Just for a sanity check print out each color in the GCT
        // fprintf(stdout, "\n------- Global Color Table -------\n");
        // for (std::vector<uint8_t> c : *colorTable) {
        //     fprintf(stdout, "Red: %X\n", c[0]);
        //     fprintf(stdout, "Green: %X\n", c[1]);
        //     fprintf(stdout, "Blue: %X\n\n", c[2]);
        // }
        // fprintf(stdout, "----------------------------------\n");
    } else {
        fprintf(stdout, "Global Color Table Not Present\n");
    }
}

void GIF::GenerateFrameMap()
{
    fprintf(stdout, "Generating Frame Map\n");
    uint8_t nextByte;
    
    // Initialize the Pixel Map

    // The pixel map will be initialized as a single vector
    // to mimic a two dimensional array, elements are accessed like so
    //(char) pixel = PixelMap.at(ROW * width) + COL
    std::vector<char> pixelMap;
    pixelMap.resize(lsd->Width * lsd->Height);

    // Initialize pixel map with blank characters
    for (int row = 0; row < lsd->Height; row++) {
        for (int col = 0; col < lsd->Width; col++) {
            pixelMap.at((row * lsd->Width) + col) = ' ';
        }
    }

    // Build up each frame for the gif
    while (true) {
        Image img = Image(file, colorTable);

        // Load Image Extenstion information before proceeding with parsing image data
        img.CheckExtensions();
        
        // Load the decompressed image data and draw the frame
        fprintf(stdout, "\nLoading Image Data...\n");
        std::string rasterData = img.LoadImageData();

        printf("%d\n", rasterData.size());

        img.UpdatePixelMap(&pixelMap, &rasterData, lsd);
        frameMap.push_back(pixelMap);
        
        fread(&nextByte, 1, sizeof(uint8_t), file);
        fseek(file, -1, SEEK_CUR);
        
        // Check if the file ended correctly (should end on 0x3B)
        if ((size_t)ftell(file) == filesize - 1) {
            if (nextByte == TRAILER)
                fprintf(stdout, "File Ended Naturally\n");
            else
                fprintf(stdout, "File ended unaturally with byte [%X]\n", nextByte);

            break;
        }
    }
}

void GIF::LoopFrames()
{
    // fprintf(stdout, "Looping Frames\n");
    std::unordered_map<int, std::string> codeTable = LZW::InitializeCodeTable(colorTable);
    // while (true) {
        for (std::vector<char> frame : frameMap) {
            
            // Initialize variables for Image Display
            int col = 0, sum = 0;
            double averageBrightness;

            for (char c : frame) {
                if (col >= lsd->Width) {
                    col = 0;
                    fprintf(stderr, "\n");
                }

                if (c == codeTable.at((int)codeTable.size() - 1)[0]) {
                    // fprintf(stdout, "%c - End of Information\n", c);
                    break;
                }

                // If for some reason a character below 0 then leave the loop
                if ((int)c < 0) 
                    break;

                std::vector<uint8_t> color = colorTable->at((int)c);

                // Get the sum of each color brightness at the current code
                for(uint8_t c : color) {
                    sum += (int)c;
                }

                averageBrightness = sum / color.size();
                // fprintf(stderr, "%s", ColorToUnicode(&color));
                fprintf(stderr, "%s", BrightnessToUnicode(averageBrightness));

                col++;
                sum = 0;
            }

            fprintf(stderr, "\n--------------------------------------------------\n");
            sleep(1);
            // system("clear");
        }
    // }
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
    fprintf(stdout, "\n------- GIF INFO -------\n");

    fprintf(stdout, "[Header]\n");
    fprintf(stdout, "\tSignature: %s\n", header->Signature);
    fprintf(stdout, "\tVersion: %s\n", header->Version);

    fprintf(stdout, "[Logical Screen Descriptor]\n");
    fprintf(stdout, "\tWidth: %d\n", lsd->Width);
    fprintf(stdout, "\tHeight: %d\n", lsd->Height);
    fprintf(stdout, "\tGlobal Color Table Flag: %d\n", (lsd->Packed >> LSDMask::GlobalColorTable) & 0x1);
    fprintf(stdout, "\tColor Resolution: %d\n", (lsd->Packed >> LSDMask::ColorResolution) & 0x07);
    fprintf(stdout, "\tSort Flag: %d\n", (lsd->Packed >> LSDMask::LSDSort) & 0x01);
    fprintf(stdout, "\tGlobal Color Table Size: %d\n", (lsd->Packed >> LSDMask::LSDSize) & 0x07);
    fprintf(stdout, "\tBackground Color Index: %d\n", lsd->BackgroundColorIndex);
    fprintf(stdout, "\tPixel Aspect Ratio: %d\n", lsd->PixelAspectRatio);

    if (lsd->Packed >> LSDMask::GlobalColorTable) {
        fprintf(stdout, "[Global Color Table]\n");
        fprintf(stdout, "\tSize: %d\n", gctd->SizeInLSD);
        fprintf(stdout, "\tNumber of Colors: %d\n", gctd->NumberOfColors);
        fprintf(stdout, "\tSize in bytes: %d\n", gctd->ByteLegth);
    }
}