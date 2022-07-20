#include "gif.h"
#include "gifmeta.h"
#include "imagemeta.h"
#include "lzw.h"
#include "errorhandler.h"
#include "Debug/debug.h"
#include "Debug/logger.h"

#include <cstdint>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <unordered_map>
#include <string.h>

GIF::GIF(FILE* _fp)
{
    this->mFile = _fp;

    // Get the file size and restore the file pointer back to position 0
    fseek(this->mFile, 0, SEEK_END);
    this->mFilesize = ftell(this->mFile);
    rewind(this->mFile);
    LOG_INFO << "Total file size:" << (this->mFilesize / 1024) << "kB" << std::endl;
   
    // Initialize class members
    this->mHeader = new GifHeader;
    this->mLsd = new LogicalScreenDescriptor;
    this->mImageData = std::vector<Image>();
    this->mFrameMap = std::vector<std::vector<char>>();
    this->mPixelMap = std::vector<char>();
    this->mPrevPixelMap = std::vector<char>();
    this->mFrameMapInitialized = false;
    this->mLSDInitialized = false;
}

void GIF::Read()
{
    LoadHeader();
    LoadLSD();
    GenerateFrameMap();
}

void GIF::LoadHeader()
{
    // Load the GIF header into memory
    fread(this->mHeader, sizeof(uint8_t), sizeof(GifHeader), this->mFile);

    // Check for a valid GIF Header
    if (!ValidHeader())
        ErrorHandler::err_n_die("Invalid GIF Format Header");
    else
        LOG_INFO << "Valid GIF Header" << std::endl;

    this->mHeaderInitialized = true;
}

void GIF::LoadLSD()
{
    if (!this->mHeaderInitialized) {
        LOG_ERROR << "Attempted to intialize Logical Screen Descriptor before header" << std::endl;
        return;
    }

    LOG_INFO << "Attempting to load Logical Screen Descriptor" << std::endl;

    //Load the LSD From GIF File 
    fread(this->mLsd, sizeof(uint8_t), sizeof(LogicalScreenDescriptor), this->mFile);

    // Check to see if the GCT flag is set
    if (this->mLsd->Packed >> LSDMask::GlobalColorTable) {
        LOG_INFO << "GCTD Present - Loading GCTD" << std::endl;

        // Load the Global Color Table Descriptor Data
        this->mGctd = new GlobalColorTableDescriptor;
        this->mGctd->SizeInLSD = (this->mLsd->Packed >> LSDMask::Size) & 0x07;
        this->mGctd->NumberOfColors = pow(2, this->mGctd->SizeInLSD + 1);
        this->mGctd->ByteLegth = 3 * this->mGctd->NumberOfColors;

        // Generate the GCT from each color present in file
        this->mColorTable = new Color[this->mGctd->NumberOfColors];
        for (int i = 0; i < this->mGctd->NumberOfColors; i++) {
            Color color = NULL_COLOR;
            fread(&color, sizeof(uint8_t), COLOR_SIZE, this->mFile);
            this->mColorTable[i] = color; 
        }

        LOG_SUCCESS << "Loaded GCTD" << std::endl;
        PrintColorTable();
    } else {
        LOG_INFO << "GCT Not present" << std::endl;
    }

    PrintHeaderInfo();
    this->mLSDInitialized = true;
    LOG_SUCCESS << "Logical Screen Descriptor Initialized" << std::endl;
}

void GIF::GenerateFrameMap()
{
    if (!this->mHeaderInitialized) {
        LOG_ERROR << "Attempted to intialize frame map before header" << std::endl;
        return;
    }
    
    if (!this->mLSDInitialized) {
        LOG_ERROR << "Attempted to intialize frame map before Logical Screen Descriptor" << std::endl;
        return;
    }

    LOG_INFO << "Generating Frame Map" << std::endl;
    uint8_t nextByte;
    
    // The pixel map will be initialized as a single vector
    // to mimic a two dimensional array, elements are accessed like so
    // (char) pixel = PixelMap.at(ROW * width) + COL
    this->mPixelMap.resize(this->mLsd->Width * this->mLsd->Height);

    // Initialize pixel map with blank characters
    for (int row = 0; row < this->mLsd->Height; row++) {
        for (int col = 0; col < this->mLsd->Width; col++) {
            this->mPixelMap.at((row * this->mLsd->Width) + col) = ' ';
        }
    }

    // Build up each frame for the gif
    while (true) {
        Image img = Image(this->mFile, this->mColorTable, this->mGctd->NumberOfColors);

        // Load Image Extenstion information before proceeding with parsing image data
        img.CheckExtensions();
        
        // Load the decompressed image data and draw the frame
        LOG_INFO << "Loading Image Data" << std::endl;
        std::string rasterData = img.LoadImageData();

        this->mPrevPixelMap = this->mPixelMap; 
        img.UpdatePixelMap(&this->mPixelMap, &this->mPrevPixelMap, &rasterData, this->mLsd);
        this->mFrameMap.push_back(this->mPixelMap);
        this->mImageData.push_back(img);

        fread(&nextByte, sizeof(uint8_t), 1, this->mFile);
        fseek(this->mFile, -1, SEEK_CUR);
        
        // Check if the file ended correctly (should end on 0x3B)
        if ((size_t)ftell(this->mFile) == this->mFilesize - 1) {
            if (nextByte == TRAILER)
                LOG_SUCCESS << "File ended naturally" << std::endl;
            else
                LOG_WARN << "File ended unaturally with byte [" << nextByte << "]" << std::endl;

            // There is nothing left to get from the file so close it
            fclose(this->mFile);
            break;
        }
    }
    
    this->mFrameMapInitialized = true;
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
    Debug::Print("\tSort Flag: %d", (this->mLsd->Packed >> LSDMask::Sort) & 0x01);
    Debug::Print("\tGlobal Color Table Size: %d", (this->mLsd->Packed >> LSDMask::Size) & 0x07);
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
