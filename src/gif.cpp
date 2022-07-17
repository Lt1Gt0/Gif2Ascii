#include "gif.h"
#include "colortounicode.h"
#include "gifmeta.h"
#include "lzw.h"
#include "errorhandler.h"
#include "Debug/debug.h"

#include <cstdint>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <unordered_map>

GIF::GIF(FILE* _fp)
{
    this->mFile = _fp;

    // Get the file size and restore the file pointer back to position 0
    fseek(this->mFile, 0, SEEK_END);
    this->mFilesize = ftell(this->mFile);
    rewind(this->mFile);
    Debug::Print("Total File Size: %.2ldkB", (this->mFilesize / 1024));
    
    LoadHeader();
    
    // Initialize class members
    this->mLsd = new LogicalScreenDescriptor;
    this->mImageData = std::vector<Image>();
    this->mFrameMap = std::vector<std::vector<char>>();
}

void GIF::LoadHeader()
{
    // Load the GIF header into memory
    this->mHeader = new GifHeader; 
    fread(this->mHeader, sizeof(uint8_t), sizeof(GifHeader), this->mFile);

    // Check for a valid GIF Header
    if (!ValidHeader())
        ErrorHandler::err_n_die("Invalid GIF Format Header");
    else
        Debug::Print("Valid GIF Header format");
}

void GIF::ReadFileDataHeaders()
{
    //Load the LSD From GIF File 
    fread(this->mLsd, sizeof(uint8_t), sizeof(LogicalScreenDescriptor), this->mFile);

    // Check to see if the GCT flag is set
    if (this->mLsd->Packed >> LSDMask::GlobalColorTable) {
        Debug::Print("Global Color Table Present\nLoading GCT Descriptor...");

        // Load the Global Color Table Descriptor Data
        this->mGctd = new GlobalColorTableDescriptor;
        this->mGctd->SizeInLSD = (this->mLsd->Packed >> LSDMask::LSDSize) & 0x07;
        this->mGctd->NumberOfColors = pow(2, this->mGctd->SizeInLSD + 1);
        this->mGctd->ByteLegth = 3 * this->mGctd->NumberOfColors;

        // Generate the GCT from each color present in file
        this->mColorTable = new Color[this->mGctd->NumberOfColors];
        for (int i = 0; i < this->mGctd->NumberOfColors; i++) {
            Color color = NULL_COLOR;
            fread(&color, sizeof(uint8_t), COLOR_SIZE, this->mFile);
            this->mColorTable[i] = color; 
        }

        Debug::Print("Successfully Loaded GCT Descriptor");
        PrintColorTable();
    } else {
        Debug::Print("Global Color Table Not Present");
    }

    PrintHeaderInfo();
}

void GIF::GenerateFrameMap()
{
    Debug::Print("Generating Frame Map");
    uint8_t nextByte;
    
    // The pixel map will be initialized as a single vector
    // to mimic a two dimensional array, elements are accessed like so
    // (char) pixel = PixelMap.at(ROW * width) + COL
    std::vector<char> pixelMap;
    pixelMap.resize(this->mLsd->Width * this->mLsd->Height);

    // Initialize pixel map with blank characters
    for (int row = 0; row < this->mLsd->Height; row++) {
        for (int col = 0; col < this->mLsd->Width; col++) {
            pixelMap.at((row * this->mLsd->Width) + col) = ' ';
        }
    }

    // Build up each frame for the gif
    while (true) {
        Image img = Image(this->mFile, this->mColorTable, this->mGctd->NumberOfColors);

        // Load Image Extenstion information before proceeding with parsing image data
        img.CheckExtensions();
        
        // Load the decompressed image data and draw the frame
        Debug::Print("\nLoading Image Data...");
        std::string rasterData = img.LoadImageData();

        img.UpdatePixelMap(&pixelMap, &rasterData, this->mLsd);
        this->mFrameMap.push_back(pixelMap);
        this->mImageData.push_back(img);

        fread(&nextByte, sizeof(uint8_t), 1, this->mFile);
        fseek(this->mFile, -1, SEEK_CUR);
        
        // Check if the file ended correctly (should end on 0x3B)
        if ((size_t)ftell(this->mFile) == this->mFilesize - 1) {
            if (nextByte == TRAILER)
                Debug::Print("File Ended Naturally");
            else
                Debug::Print("File ended unaturally with byte [%X]", nextByte);

            // There is nothing left to get from the file so close it
            fclose(this->mFile);
            break;
        }
    }
}

void GIF::LoopFrames()
{
    // fprintf(stdout, "Looping Frames\n");
    std::unordered_map<int, std::string> codeTable = LZW::InitializeCodeTable(this->mGctd->NumberOfColors);
    system("clear");
    while (true) {
        int frameIdx = 0;
        for (std::vector<char> frame : this->mFrameMap) {
            
            // Initialize variables for Image Display
            int col = 0, sum = 0;
            double averageBrightness;

            for (char c : frame) {
                if (col >= this->mLsd->Width) {
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

                Color color = this->mColorTable[(int)c];

                // Get the sum of each color brightness at the current code
                sum += color.Red;
                sum += color.Green;
                sum += color.Blue;

                averageBrightness = (float)sum / (float)COLOR_SIZE;
                fprintf(stderr, "%s", ColorToUnicode(color));
                // fprintf(stderr, "%s", BrightnessToUnicode(averageBrightness));

                col++;
                sum = 0;
            }

            // Debug::Print("\n--------------------------------------------------");
            //sleep((double) this->mImageData[frameIdx].extensions->GraphicsControl->DelayTime / 1000);
            sleep(1);
            frameIdx++;
            system("clear");
        }
    }
}

bool GIF::ValidHeader()
{
    for (int i = 0; i < 6; i++) {
        if (this->mHeader->Signature[i] != GIF_MAGIC_0[i]
         && this->mHeader->Signature[i] != GIF_MAGIC_1[i]) {
            return false;
        } 
    }

    return true;
}

void GIF::PrintHeaderInfo()
{   
    Debug::Print("\n------- GIF INFO -------");

    Debug::Print("[Header]");
    Debug::Print("\tSignature: %s", this->mHeader->Signature);
    Debug::Print("\tVersion: %s", this->mHeader->Version);

    Debug::Print("[Logical Screen Descriptor]");
    Debug::Print("\tWidth: %d", this->mLsd->Width);
    Debug::Print("\tHeight: %d", this->mLsd->Height);
    Debug::Print("\tGlobal Color Table Flag: %d", (this->mLsd->Packed >> LSDMask::GlobalColorTable) & 0x1);
    Debug::Print("\tColor Resolution: %d", (this->mLsd->Packed >> LSDMask::ColorResolution) & 0x07);
    Debug::Print("\tSort Flag: %d", (this->mLsd->Packed >> LSDMask::LSDSort) & 0x01);
    Debug::Print("\tGlobal Color Table Size: %d", (this->mLsd->Packed >> LSDMask::LSDSize) & 0x07);
    Debug::Print("\tBackground Color Index: %d", this->mLsd->BackgroundColorIndex);
    Debug::Print("\tPixel Aspect Ratio: %d", this->mLsd->PixelAspectRatio);

    if (this->mLsd->Packed >> LSDMask::GlobalColorTable) {
        Debug::Print("[Global Color Table]");
        Debug::Print("\tSize: %d", this->mGctd->SizeInLSD);
        Debug::Print("\tNumber of Colors: %d", this->mGctd->NumberOfColors);
        Debug::Print("\tSize in bytes: %d", this->mGctd->ByteLegth);
    }
}

void GIF::PrintColorTable()
{
    Debug::Print("\n------- Global Color Table -------");
    for (int i = 0; i < this->mGctd->NumberOfColors; i++) {
        Debug::Print("Red: %X", this->mColorTable[i].Red);
        Debug::Print("Green: %X", this->mColorTable[i].Green);
        Debug::Print("Blue: %X\n", this->mColorTable[i].Blue);
    }
    Debug::Print("----------------------------------");
}
