#include "gif.hpp"
#include "gifmeta.hpp"
#include "imagemeta.hpp"
#include "lzw.hpp"
#include "utils/logger.hpp"
#include "utils/error.hpp"

#include <cstdint>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <unordered_map>
#include <string.h>

GIF::GIF(const char* _filepath)
{
    this->mFile = fopen(_filepath, "rb");

    if (this->mFile == NULL)
        error(Severity::high, "Error opening file:", _filepath);
    else
        logger.Log(DEBUG, "Opened [%s]", _filepath); 

    // Get the file size and restore the file pointer back to position 0
    fseek(this->mFile, 0, SEEK_END);
    this->mFilesize = ftell(this->mFile);
    rewind(this->mFile);
    logger.Log(INFO, "Total file size: %dkB", this->mFilesize / 1024);
   
    // Initialize class members
    this->mHeader = {};
    this->mLsd = {};
    this->mImageData = std::vector<Image>();
    this->mFrameMap = std::vector<std::vector<char>>();
    this->mPixelMap = std::vector<char>();
    this->mPrevPixelMap = std::vector<char>();
    this->mFrameMapInitialized = false;
    this->mLSDInitialized = false;
}

void GIF::Read()
{
    logger.Log(DEBUG, "Reading GIF Information");
    LoadHeader();
    LoadLSD();
    GenerateFrameMap();
    logger.Log(DEBUG, "Read GIF Information");
}

void GIF::LoadHeader()
{
    // Load the GIF header into memory
    fread(&this->mHeader, sizeof(uint8_t), sizeof(GifHeader), this->mFile);

    logger.Log(TRACE, "Checking for valid GIF Header");
    if (!ValidHeader())
        error(Severity::high, "GIF:", "Invalid Format Header");
    else
        logger.Log(DEBUG, "Valid GIF Header");

    this->mHeaderInitialized = true;
}

void GIF::LoadLSD()
{
    if (!this->mHeaderInitialized)
        error(Severity::medium, "GIF:", "Attempted to initialize frame map before header");

    logger.Log(TRACE, "Loading Logical Screen Descriptor");

    //Load the LSD From GIF File 
    fread(&this->mLsd, sizeof(uint8_t), sizeof(LogicalScreenDescriptor), this->mFile);

    logger.Log(TRACE, "Checking for GCT flag");
    if (this->mLsd.Packed >> (int)LSDMask::GlobalColorTable) {
        logger.Log(DEBUG, "GCTD Present - Loading GCTD");

        // Load the Global Color Table Descriptor Data
        this->mGctd = {};
        this->mGctd.SizeInLSD = (this->mLsd.Packed >> (uint8_t)LSDMask::Size) & 0x07;
        this->mGctd.NumberOfColors = pow(2, this->mGctd.SizeInLSD + 1);
        this->mGctd.ByteLegth = 3 * this->mGctd.NumberOfColors;

        // Generate the GCT from each color present in file
        this->mColorTable = new Color[this->mGctd.NumberOfColors];
        for (int i = 0; i < this->mGctd.NumberOfColors; i++) {
            Color color = NULL_COLOR;
            fread(&color, sizeof(uint8_t), COLOR_SIZE, this->mFile);
            this->mColorTable[i] = color; 
        }

        logger.Log(SUCCESS, "Loaded GCTD");
        PrintColorTable();
    } else {
        logger.Log(DEBUG, "GCT Not present");
    }

    PrintHeaderInfo();
    this->mLSDInitialized = true;
    logger.Log(SUCCESS, "Logical Screen Descriptor Initialized");
}

void GIF::GenerateFrameMap()
{
    if (!this->mHeaderInitialized)
        error(Severity::medium, "GIF:", "Attempted to initialize frame map before header");
    
    if (!this->mLSDInitialized)
        error(Severity::medium, "GIF:", "Attempted to initialize frame map before Logical Screen Descriptor");

    logger.Log(TRACE, "Generating Frame Map");
    uint8_t nextByte;
    
    // The pixel map will be initialized as a single vector
    // to mimic a two dimensional array, elements are accessed like so
    // (char) pixel = PixelMap.at(ROW * width) + COL
    this->mPixelMap.resize(this->mLsd.Width * this->mLsd.Height);

    // Initialize pixel map with blank characters
    for (int row = 0; row < this->mLsd.Height; row++) {
        for (int col = 0; col < this->mLsd.Width; col++) {
            this->mPixelMap.at((row * this->mLsd.Width) + col) = ' ';
        }
    }

    // Build up each frame for the gif
    while (true) {
        Image img = Image(this->mFile, this->mColorTable, this->mGctd.NumberOfColors);

        // Load Image Extenstion information before proceeding with parsing image data
        img.CheckExtensions();
        
        // Load the decompressed image data and draw the frame
        logger.Log(DEBUG, "Loading Image Data");
        std::string rasterData = img.LoadImageData();

        this->mPrevPixelMap = this->mPixelMap; 
        img.UpdatePixelMap(&this->mPixelMap, &this->mPrevPixelMap, &rasterData, &this->mLsd);
        this->mFrameMap.push_back(this->mPixelMap);
        this->mImageData.push_back(img);

        fread(&nextByte, sizeof(uint8_t), 1, this->mFile);
        fseek(this->mFile, -1, SEEK_CUR);
        
        // Check if the file ended correctly (should end on 0x3B)
        if ((size_t)ftell(this->mFile) == this->mFilesize - 1) {
            if (nextByte == TRAILER)
                logger.Log(SUCCESS, "File ended naturally");
            else
                logger.Log(WARNING, "File ended unaturally with byte [%c]", nextByte);

            // There is nothing left to get from the file so close it
            fclose(this->mFile);
            break;
        }
    }
    
    this->mFrameMapInitialized = true;
}

bool GIF::ValidHeader()
{
    for (int i = 0; i < 3; i++) {
        if (this->mHeader.Signature[i] != gifSignature[i])
           return false; 
    }

    for (int i = 0; i < 3; i++) {
        if (this->mHeader.Version[i] != gif87a[i]
         && this->mHeader.Version[i] != gif89a[i]) {
            return false;
        } 
    }

    return true;
}

void GIF::SigIntHandler(int sig)
{
    system("clear");        
    exit(0);
}

void GIF::PrintHeaderInfo()
{   
    logger.Log(DEBUG, "------- GIF INFO -------");

    logger.Log(DEBUG, "[Header]");
    logger.Log(DEBUG, "\tSignature: %s", this->mHeader.Signature);
    logger.Log(DEBUG, "\tVersion: %s", this->mHeader.Version);

    logger.Log(DEBUG, "[Logical Screen Descriptor]");
    logger.Log(DEBUG, "\tWidth: %d", this->mLsd.Width);
    logger.Log(DEBUG, "\tHeight: %d", this->mLsd.Height);
    logger.Log(DEBUG, "\tGlobal Color Table Flag: %d", (this->mLsd.Packed >> (uint8_t)LSDMask::GlobalColorTable) & 0x1);
    logger.Log(DEBUG, "\tColor Resolution: %d", (this->mLsd.Packed >> (uint8_t)LSDMask::ColorResolution) & 0x07);
    logger.Log(DEBUG, "\tSort Flag: %d", (this->mLsd.Packed >> (uint8_t)LSDMask::Sort) & 0x01);
    logger.Log(DEBUG, "\tGlobal Color Table Size: %d", (this->mLsd.Packed >> (uint8_t)LSDMask::Size) & 0x07);
    logger.Log(DEBUG, "\tBackground Color Index: %d", this->mLsd.BackgroundColorIndex);
    logger.Log(DEBUG, "\tPixel Aspect Ratio: %d", this->mLsd.PixelAspectRatio);

    if (this->mLsd.Packed >> (uint8_t)LSDMask::GlobalColorTable) {
        logger.Log(DEBUG, "[Global Color Table]");
        logger.Log(DEBUG, "\tSize: %d", this->mGctd.SizeInLSD);
        logger.Log(DEBUG, "\tNumber of Colors: %d", this->mGctd.NumberOfColors);
        logger.Log(DEBUG, "\tSize in bytes: %d", this->mGctd.ByteLegth);
    }
}

void GIF::PrintColorTable()
{
    logger.Log(DEBUG, "------- Global Color Table -------");
    for (int i = 0; i < this->mGctd.NumberOfColors; i++) {
        logger.Log(DEBUG, "Red: %X", this->mColorTable[i].Red);
        logger.Log(DEBUG, "Green: %X", this->mColorTable[i].Green);
        logger.Log(DEBUG, "Blue: %X", this->mColorTable[i].Blue);
    }
    logger.Log(DEBUG, "----------------------------------");
}
