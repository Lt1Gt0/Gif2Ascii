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
#include <cstring>
#include <vector>

template<class T>
using Vec = std::vector<T>;

namespace GIF
{
    File::File(std::string filepath) : mPath(filepath)
    {
        mInStream = std::ifstream(mPath, std::ifstream::in | std::ifstream::binary);
        mFileBuffer = mInStream.rdbuf();

        if (!mFileBuffer)
            error(Severity::high, "GIF:", "Unable to open file - ", mPath);
        else 
            LOG_SUCCESS << "Opened [" << mPath << "]" << std::endl;

        // Get the file size and restore the file pointer back to position 0
        mFileSize = mFileBuffer->pubseekoff(0, mInStream.end, mInStream.in);
        mFileBuffer->pubseekpos(0, mInStream.in);
        mCurrentFilePos = mInStream.tellg(); 
        LOG_DEBUG << "Total file size: " << (mFileSize / 1024) << "kB (" << mFileSize << " B)" << std::endl;

        Read();

        mFileBuffer->close();
        mInStream.close();

        LOG_DEBUG << "Buffered GIF Data" << std::endl;
    }

    File::~File() {}

    Status File::ParseHeader()
    {
        if (!mFileBuffer->is_open())
            error(Severity::high, "File Buffer:", "Buffer was never initialized or was closed");

        // Load the GIF header into memory
        mInStream.read(reinterpret_cast<char*>(&mHeader), sizeof(byte) * sizeof(Header));
        
        if (!(std::strncmp(mHeader.signature, GIF_SIGNATURE, 3)) && 
            !(strncmp(mHeader.version, GIF_87A, 3) || strncmp(mHeader.version, GIF_89A, 3)))
            return Status::failure;

        LOG_SUCCESS << "Parsed GIF Header" << std::endl;
        return Status::success;
    }

    void File::Read()
    {
        LOG_INFO << "Reading GIF Information" << std::endl;
        
        if (ParseHeader() == Status::failure)
            error(Severity::high, "GIF:", "Unable to parse header");

        ParseLSD();
        GenerateFrameMap();

        LOG_SUCCESS << "Read GIF Information" << std::endl;
    }

    Status File::ParseLSD()
    {
        LOG_INFO << "Attempting to load Logical Screen Descriptor" << std::endl;

        //Load the LSD From GIF File 
        mInStream.read(reinterpret_cast<char*>(&mLSD), sizeof(byte) * sizeof(LSD));

        // Check to see if the GCT flag is set
        if (mLSD.packed >> (int)LSDMask::GlobalColorTable) {
            LOG_INFO << "GCTD Present - Loading GCTD" << std::endl;

            mGCTD = {};
            mGCTD.sizeInLSD = (mLSD.packed >> (byte)LSDMask::Size) & 0x07;
            mGCTD.colorCount = pow(2, mGCTD.sizeInLSD + 1);
            mGCTD.byteLength = 3 * mGCTD.colorCount;

            // Generate the GCT from each color present in file
            mGCT = new Color[mGCTD.colorCount];
            for (int i = 0; i < mGCTD.colorCount; i++) {
                Color color = {};
                mInStream.read(reinterpret_cast<char*>(&color), sizeof(byte) * sizeof(Color));
                mGCT[i] = color; 
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
        Vec<char> pixelMap(mLSD.width * mLSD.height, 0);
        Vec<char> prevPixelMap = pixelMap;

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
            mFrameMap.push_back(pixelMap);
            mImageData.push_back((void*)&img);

            nextByte = mInStream.peek();
            //mFileBuffer->sgetc();
            //fseek(mFP, -1, SEEK_CUR);
            
            // Check if the file ended correctly (should end on 0x3B)
            if ((size_t)mInStream.tellg() == mFileSize - 1) {
                if (nextByte == TRAILER)
                    LOG_SUCCESS << "File ended naturally" << std::endl;
                else
                    LOG_WARN << "File ended unaturally with byte [" << nextByte << "]" << std::endl;

                // There is nothing left to get from the file so close it
                mInStream.close();
                break;
            }
        }

        LOG_SUCCESS << "Generated Frame Map" << std::endl;
        return Status::success;
    }

    void File::DumpInfo(std::string dumpPath)
    {
        std::ofstream dump (dumpPath);
        
        if (!dump.is_open())
            error(Severity::medium, "GIF:", "Unable to open dump file");
        
        // Header information
        dump << "File: " << mPath << std::endl;
        dump << "Version: " << mHeader.version[0] << mHeader.version[1] << mHeader.version[2] << std::endl;

        // Logical Screen Descriptor
        dump << "LSD Width: " << (int)mLSD.width << std::endl; 
        dump << "LSD Height: " << (int)mLSD.height << std::endl; 
        dump << "GCTD Present: " << (int)((mLSD.packed >> (byte)LSDMask::GlobalColorTable) & 0x1) << std::endl;
        dump << "LSD Background Color Index: " << (int)mLSD.backgroundColorIndex << std::endl; 
        dump << "LSD Pixel Aspect Ratio: " << (int)mLSD.pixelAspectRatio << std::endl; 

        // GCTD / GCT
        if ((mLSD.packed >> (byte)LSDMask::GlobalColorTable) & 0x1) {
            dump << "GCT Size: " << (int)mGCTD.sizeInLSD << std::endl;
            dump << "GCT Color Count: " << (int)mGCTD.colorCount << std::endl;
            dump << "GCT Size in bytes: " << (int)mGCTD.byteLength << std::endl;
            
            dump.setf(std::ios::hex, std::ios::basefield);    
            for (int i = 0; i < mGCTD.colorCount; i++) {
                    dump << "\tRed: " << std::uppercase << (int)mGCT[i].Red << std::endl;
                    dump << "\tGreen: " << std::uppercase << (int)mGCT[i].Green << std::endl;
                    dump << "\tBlue: " << std::uppercase << (int)mGCT[i].Blue << std::endl;
                    dump << std::endl;
            } 
            dump.unsetf(std::ios::hex);
        }
            
        dump.close();
    }
}
