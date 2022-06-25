#include "gif.h"
#include "colortounicode.h"
#include "lzw.h"
#include "errorhandler.h"
#include "Debug/debug.h"

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
    Debug::Print("Total File Size: %.2ldkB", (filesize / 1024));

    // Load the GIF header into memory
    this->header = new GifHeader; 
    fread(header, 1, sizeof(GifHeader), file);

    // Check for a valid GIF Header
    if (!ValidHeader())
        ErrorHandler::err_n_die("Invalid GIF Format Header");
    else
        Debug::Print("Valid GIF Header format");

    this->lsd = new LogicalScreenDescriptor;
    this->imageData = std::vector<Image>();
    this->frameMap = std::vector<std::vector<char>>();
}

void GIF::ReadFileDataHeaders()
{
    fread(lsd, sizeof(uint8_t), sizeof(LogicalScreenDescriptor), file);

    // Check to see if the GCT flag is set
    if (lsd->Packed >> LSDMask::GlobalColorTable) {
        Debug::Print("Global Color Table Present\nLoading GCT Descriptor...");

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
            for (int j = 0; j < (int)color.size(); j++) {
                fread(&color[j], 1, sizeof(char), file);
            }

            colorTable->push_back(color);
            color.clear();
            color.resize(3);
        }
        Debug::Print("Successfully Loaded GCT Descriptor");

        // Just for a sanity check print out each color in the GCT
        Debug::Print("\n------- Global Color Table -------");
        for (std::vector<uint8_t> c : *colorTable) {
            Debug::Print("Red: %X", c[0]);
            Debug::Print("Green: %X", c[1]);
            Debug::Print("Blue: %X\n", c[2]);
        }
        Debug::Print("----------------------------------");
    } else {
        Debug::Print("Global Color Table Not Present");
    }

    PrintHeaderInfo();
}

void GIF::GenerateFrameMap()
{
    Debug::Print("Generating Frame Map");
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
        Debug::Print("\nLoading Image Data...");
        std::string rasterData = img.LoadImageData();

        img.UpdatePixelMap(&pixelMap, &rasterData, lsd);
        frameMap.push_back(pixelMap);
        imageData.push_back(img);

        fread(&nextByte, 1, sizeof(uint8_t), file);
        fseek(file, -1, SEEK_CUR);
        
        // Check if the file ended correctly (should end on 0x3B)
        if ((size_t)ftell(file) == filesize - 1) {
            if (nextByte == TRAILER)
                Debug::Print("File Ended Naturally");
            else
                Debug::Print("File ended unaturally with byte [%X]", nextByte);

            // There is nothing left to get from the file so close it
            fclose(this->file);
            break;
        }
    }
}

void GIF::LoopFrames()
{
    // fprintf(stdout, "Looping Frames\n");
    std::unordered_map<int, std::string> codeTable = LZW::InitializeCodeTable(colorTable);
    system("clear");
    while (true) {
        int frameIdx = 0;
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
                    Debug::Print("%c - End of Information", c);
                    break;
                }

                // If for some reason a character below 0 then leave the loop
                if ((int)c < 0) 
                    break;

                std::vector<uint8_t> color = colorTable->at((int)c);

                // Get the sum of each color brightness at the current code
                for(uint8_t brightness : color) {
                    sum += brightness;
                }

                averageBrightness = sum / color.size();
                fprintf(stderr, "%s", ColorToUnicode(&color));
                // fprintf(stderr, "%s", BrightnessToUnicode(averageBrightness));

                col++;
                sum = 0;
            }

            // Debug::Print("\n--------------------------------------------------");
            sleep((double) imageData[frameIdx].extensions->GraphicsControl->DelayTime / 50);
            frameIdx++;
            system("clear");
        }
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
    Debug::Print("\n------- GIF INFO -------");

    Debug::Print("[Header]");
    Debug::Print("\tSignature: %s", header->Signature);
    Debug::Print("\tVersion: %s", header->Version);

    Debug::Print("[Logical Screen Descriptor]");
    Debug::Print("\tWidth: %d", lsd->Width);
    Debug::Print("\tHeight: %d", lsd->Height);
    Debug::Print("\tGlobal Color Table Flag: %d", (lsd->Packed >> LSDMask::GlobalColorTable) & 0x1);
    Debug::Print("\tColor Resolution: %d", (lsd->Packed >> LSDMask::ColorResolution) & 0x07);
    Debug::Print("\tSort Flag: %d", (lsd->Packed >> LSDMask::LSDSort) & 0x01);
    Debug::Print("\tGlobal Color Table Size: %d", (lsd->Packed >> LSDMask::LSDSize) & 0x07);
    Debug::Print("\tBackground Color Index: %d", lsd->BackgroundColorIndex);
    Debug::Print("\tPixel Aspect Ratio: %d", lsd->PixelAspectRatio);

    if (lsd->Packed >> LSDMask::GlobalColorTable) {
        Debug::Print("[Global Color Table]");
        Debug::Print("\tSize: %d", gctd->SizeInLSD);
        Debug::Print("\tNumber of Colors: %d", gctd->NumberOfColors);
        Debug::Print("\tSize in bytes: %d", gctd->ByteLegth);
    }
}