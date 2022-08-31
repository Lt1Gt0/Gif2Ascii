#include "gif.h"
#include "common.h"
#include "logger.h"
#include "gifmeta.h"
#include "lzw.h"

#include <cstdint>
#include <unistd.h>
#include <stdio.h>
#include <tgmath.h>
#include <string.h>
#include <unordered_map>
#include <string.h>
#include <vector>

namespace GIF
{
    File::File(const char* filepath) : mPath(filepath)
    {
        this->mFP = fopen(this->mPath, "rb");

        if (this->mFP == nullptr)
            Debug::error(Severity::high, "GIF:", "Unable to open file - ", this->mPath);
        else
            LOG_SUCCESS << "Opened [" << this->mPath << "]" << std::endl;

        // Get the file size and restore the file pointer back to position 0
        fseek(this->mFP, 0, SEEK_END);
        this->mFileSize = ftell(this->mFP);
        rewind(this->mFP);
        LOG_INFO << "Total file size: " << (this->mFileSize / 1024) << "kB" << std::endl;
    }

    File::~File() {}

    Status File::ParseHeader()
    {
        // Load the GIF header into memory
        fread(&this->mHeader, sizeof(byte), sizeof(Header), this->mFP);

        if (!(strncmp(this->mHeader.signature, GIF_SIGNATURE, 3)) && 
            !(strncmp(this->mHeader.version, GIF_87A, 3) || strncmp(this->mHeader.version, GIF_89A, 3)))
            return Status::failure;

        LOG_SUCCESS << "Parsed GIF Header" << std::endl;
        return Status::success;
    }

    void File::Read()
    {
        LOG_INFO << "Reading GIF Information" << std::endl;
        
        if (ParseHeader() == Status::failure)
            Debug::error(Severity::high, "GIF:", "Unable to parse header");

        ParseLSD();
        GenerateFrameMap();

        LOG_SUCCESS << "Read GIF Information" << std::endl;
    }

    Status File::ParseLSD()
    {
        LOG_INFO << "Attempting to load Logical Screen Descriptor" << std::endl;

        //Load the LSD From GIF File 
        fread(&this->mLSD, sizeof(byte), sizeof(LSD), this->mFP);

        // Check to see if the GCT flag is set
        if (this->mLSD.packed >> (int)LSDMask::GlobalColorTable) {
            LOG_INFO << "GCTD Present - Loading GCTD" << std::endl;

            this->mGCTD = {};
            this->mGCTD.sizeInLSD = (this->mLSD.packed >> (byte)LSDMask::Size) & 0x07;
            this->mGCTD.colorCount = pow(2, this->mGCTD.sizeInLSD + 1);
            this->mGCTD.byteLength = 3 * this->mGCTD.colorCount;

            // Generate the GCT from each color present in file
            this->mGCT = new Color[this->mGCTD.colorCount];
            for (int i = 0; i < this->mGCTD.colorCount; i++) {
                Color color = {};
                fread(&color, sizeof(byte), sizeof(Color), this->mFP);
                this->mGCT[i] = color; 
            }

            LOG_SUCCESS << "Loaded GCTD" << std::endl;
        } 

        LOG_SUCCESS << "Logical Screen Descriptor Initialized" << std::endl;
        return Status::success;
    }



    Status File::GenerateFrameMap()
    {
        LOG_INFO << "Generating Frame Map" << std::endl;
        
        // The pixel map will be initialized as a single vector
        // to mimic a two dimensional array, elements are accessed like so
        // (char) pixel = PixelMap.at(ROW * width) + COL
        std::vector<char> pixelMap(this->mLSD.width * this->mLSD.height, 0);
        std::vector<char> prevPixelMap = pixelMap;

        // Build up each frame for the gif
        byte nextByte = 0;
        while (true) {
            Image img = Image();

            // Load Image Extenstion information before proceeding with parsing image data
            img.CheckExtensions(this);
            
            // Load the decompressed image data and draw the frame
            LOG_INFO << "Loading Image Data" << std::endl;
            std::string rasterData = img.LoadData(this);

            prevPixelMap = pixelMap; 
            img.UpdatePixelMap(this, rasterData, pixelMap, prevPixelMap);
            this->mFrameMap.push_back(pixelMap);
            this->mImageData.push_back((void*)&img);

            fread(&nextByte, sizeof(byte), 1, this->mFP);
            fseek(this->mFP, -1, SEEK_CUR);
            
            // Check if the file ended correctly (should end on 0x3B)
            if ((size_t)ftell(this->mFP) == this->mFileSize - 1) {
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
}
