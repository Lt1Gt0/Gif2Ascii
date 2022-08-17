#include "gif.h"
#include "common.h"
#include "gifmeta.h"
#include "logger.h"

#include <ios>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <tgmath.h>

namespace GIF
{
    File::File(const char* path) : mPath(path) {}

    void File::OpenFile()
    {
        this->mFP = fopen(this->mPath, "rb");

        if (this->mFP == nullptr)
            Debug::error(Severity::high, "GIF:", "Unable to open file");
        else
            LOG_SUCCESS << "Opened [" << this->mPath << "]" << std::endl;

        fseek(this->mFP, 0, SEEK_END);
        this->mSize = ftell(this->mFP);
        rewind(this->mFP);
        LOG_INFO << "Total File Size: " << (this->mSize) / 1024 << "kB" << std::endl;
    }

    Status File::ParseHeader()
    {
        // Load the GIF header into memory
        fread(&this->mHeader, sizeof(byte), sizeof(Header), this->mFP);

        // Check for a valid GIF Header
        for (int i = 0; i < 3; i++) {
            if (this->mHeader.signature[i] != GIF_SIGNATURE[i])
                return Status::failure;
        } 

        for (int i = 0; i < 3; i++) {
            if (this->mHeader.version[i] != GIF_87A[i]
             && this->mHeader.version[i] != GIF_89A[i]) {
                return Status::failure;
            }
        }

        LOG_SUCCESS << "Parsed GIF Header" << std::endl;
        return Status::success;
    }

    void File::Read()
    {
        LOG_INFO << "Reading file information" << std::endl;
        ParseLSD();
        GenerateFrameMap();
        LOG_SUCCESS << "Read File Information" << std::endl;
    }

    Status File::ParseLSD()
    {
        LOG_INFO << "Attempting to parse LSD" << std::endl;

        this->mLSD = {};
        fread(&this->mLSD, sizeof(byte), sizeof(LSD), this->mFP);
        if (this->mLSD.packed >> (int)LSDMask::GlobalColorTable) {
            LOG_INFO << "GCTD Present - Loading GCTD" << std::endl;

            this->mGCTD = {};
            this->mGCTD.sizeInLSD = (this->mLSD.packed >> (byte)LSDMask::Size) & 0x7;
            this->mGCTD.colorCount = pow(2, this->mGCTD.sizeInLSD + 1);
            this->mGCTD.byteLength = 3 * this->mGCTD.colorCount;            

            // generate GCT
            this->mGCT = new Color[this->mGCTD.colorCount];
            for (int i = 0; i < this->mGCTD.colorCount; i++) {
                Color color = {};
                fread(&color, sizeof(byte), sizeof(Color), this->mFP);
                this->mGCT[i] = color;
            } 

            LOG_SUCCESS << "Loaded GCTD" << std::endl;
        }
        

        LOG_SUCCESS << "Parsed LSD" << std::endl;
        return Status::success;
    }

    Status File::GenerateFrameMap()
    {
        LOG_INFO << "Generating Frame Map" << std::endl;
       
        this->mPixelMap = new char*[this->mLSD.height];
        for (int row = 0; row < this->mLSD.height; row++) {
            this->mPixelMap[row] = new char[this->mLSD.width]; 
            for (int col = 0; col < this->mLSD.width; col++) {
                this->mPixelMap[row][col] = ' ';
            }
        } 

        byte nextByte = 0;
        while (true) {
            Image img = Image(this);

            // Load Image Extenstion information before proceeding with parsing image data
            img.CheckExtensions();
            
            // Load the decompressed image data and draw the frame
            LOG_INFO << "Loading Image Data" << std::endl;
            std::string rasterData = img.LoadData();

            this->mPrevPixelMap = this->mPixelMap; 
            img.UpdatePixelMap(rasterData, this->mPixelMap, this->mPrevPixelMap);
            this->mFrameMap.push_back(this->mPixelMap);
            this->mImageData.push_back((void*)&img);

            fread(&nextByte, sizeof(byte), 1, this->mFP);
            fseek(this->mFP, -1, SEEK_CUR);
            
            // Check if the file ended correctly (should end on 0x3B)
            if ((size_t)ftell(this->mFP) == this->mSize - 1) {
                if (nextByte == TRAILER)
                    LOG_SUCCESS << "File ended naturally" << std::endl;
                else
                    LOG_WARN << "File ended unaturally with byte [" << nextByte << "]" << std::endl;

                // There is nothing left to get from the file so close it
                fclose(this->mFP);
                break;
            }
        }
    
        LOG_SUCCESS << "Generated Frame Map" << std::endl;
        return Status::success;
    }

    void File::DumpInfo(const char* path)
    {
        std::ofstream dump (path);
        
        if (!dump.is_open())
            Debug::error(Severity::medium, "GIF:", "Unable to open dump file");
        
        /* Header information */ 
        dump << "File: " << this->mPath << std::endl;
        dump << "Version: " << this->mHeader.version[0] << this->mHeader.version[1] << this->mHeader.version[2] << std::endl;

        /* Logical Screen Descriptor */
        dump << "LSD Width: " << (int)this->mLSD.width << std::endl; 
        dump << "LSD Height: " << (int)this->mLSD.height << std::endl; 
        dump << "GCTD Present: " << (int)((this->mLSD.packed >> (byte)LSDMask::GlobalColorTable) & 0x1) << std::endl;
        dump << "LSD Background Color Index: " << (int)this->mLSD.backgroundColorIndex << std::endl; 
        dump << "LSD Pixel Aspect Ratio: " << (int)this->mLSD.pixelAspectRatio << std::endl; 

        /* GCTD / GCT */
        if ((this->mLSD.packed >> (byte)LSDMask::GlobalColorTable) & 0x1) {
            dump << "GCT Size: " << (int)this->mGCTD.sizeInLSD << std::endl;
            dump << "GCT Color Count: " << (int)this->mGCTD.colorCount << std::endl;
            dump << "GCT Size in bytes: " << (int)this->mGCTD.byteLength << std::endl;
            
            dump.setf(std::ios::hex, std::ios::basefield);    
            for (int i = 0; i < this->mGCTD.colorCount; i++) {
                    dump << "\tRed: " << std::uppercase << (int)this->mGCT[i].Red << std::endl;
                    dump << "\tGreen: " << std::uppercase << (int)this->mGCT[i].Green << std::endl;
                    dump << "\tBlue: " << std::uppercase << (int)this->mGCT[i].Blue << std::endl;
                    dump << std::endl;
            } 
            dump.unsetf(std::ios::hex);
        }
            
        dump.close();
    }

    Status Initialize(File* gif)
    {
        gif->OpenFile(); 

        if (gif->ParseHeader() == Status::failure)
            Debug::error(Severity::high, "GIF:", "Unable to parse header");

        LOG_SUCCESS << "Initialized GIF File" << std::endl;
        return Status::success;
    }

    int SigIntHandler(int sig)
    {
        system("clear");
        exit(0);
    }
}
