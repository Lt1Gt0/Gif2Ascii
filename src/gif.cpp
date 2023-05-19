#include "gif.hpp"
#include "common.hpp"
#include "logger.hpp"
#include "gifmeta.hpp"
#include "lzw.hpp"

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
        
        if (!(std::strncmp(mHeader.Signature, GIF_SIGNATURE, 3)) && 
            !(strncmp(mHeader.Version, GIF_87A, 3) || strncmp(mHeader.Version, GIF_89A, 3)))
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
        if (mLSD.Packed >> (int)LSDMask::GlobalColorTable) {
            LOG_INFO << "GCTD Present - Loading GCTD" << std::endl;

            mGCTD = {};
            mGCTD.SizeInLSD = (mLSD.Packed >> (byte)LSDMask::Size) & 0x07;
            mGCTD.ColorCount = pow(2, mGCTD.SizeInLSD + 1);
            mGCTD.ByteLength = 3 * mGCTD.ColorCount;

            // Generate the GCT from each color present in file
            mGCT = new Color[mGCTD.ColorCount];
            for (int i = 0; i < mGCTD.ColorCount; i++) {
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
        Vec<char> pixelMap(mLSD.Width * mLSD.Height, 0);
        Vec<char> prevPixelMap = pixelMap;

        // Build up each frame for the gif
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
            byte nextByte = mInStream.peek();
            
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
        dump << "Version: " << mHeader.Version[0] << mHeader.Version[1] << mHeader.Version[2] << std::endl;

        // Logical Screen Descriptor
        dump << "LSD Width: " << (int)mLSD.Width << std::endl; 
        dump << "LSD Height: " << (int)mLSD.Height << std::endl; 
        dump << "GCTD Present: " << (int)((mLSD.Packed >> (byte)LSDMask::GlobalColorTable) & 0x1) << std::endl;
        dump << "LSD Background Color Index: " << (int)mLSD.BackgroundColorIndex << std::endl; 
        dump << "LSD Pixel Aspect Ratio: " << (int)mLSD.PixelAspectRatio << std::endl; 

        // GCTD / GCT
        if ((mLSD.Packed >> (byte)LSDMask::GlobalColorTable) & 0x1) {
            dump << "GCT Size: " << (int)mGCTD.SizeInLSD << std::endl;
            dump << "GCT Color Count: " << (int)mGCTD.ColorCount << std::endl;
            dump << "GCT Size in bytes: " << (int)mGCTD.ByteLength << std::endl;
            
            dump.setf(std::ios::hex, std::ios::basefield);    
            for (int i = 0; i < mGCTD.ColorCount; i++) {
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
