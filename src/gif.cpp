#include "gif.hpp"
#include "utils/error.hpp"
#include "utils/logger.hpp"
#include "utils/types.hpp"
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
            logger.Log(SUCCESS, "Opened [%s]", mPath.c_str());

        // Get the file size and restore the file pointer back to position 0
        mFileSize = mFileBuffer->pubseekoff(0, mInStream.end, mInStream.in);
        mFileBuffer->pubseekpos(0, mInStream.in);
        mCurrentFilePos = mInStream.tellg(); 
        logger.Log(DEBUG, "Total file size: %dkB (%db)", mFileSize / 1024, mFileSize);

        Read();

        mFileBuffer->close();
        mInStream.close();

        logger.Log(SUCCESS, "Buffered GIF Data");
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

        logger.Log(SUCCESS, "Parsed GIF Header");
        return Status::success;
    }

    void File::Read()
    {
        logger.Log(INFO, "Reading GIF Information");
        
        if (ParseHeader() == Status::failure)
            error(Severity::high, "GIF:", "Unable to parse header");

        ParseLSD();
        GenerateFrameMap();

        logger.Log(SUCCESS, "Read GIF Information");
    }

    Status File::ParseLSD()
    {
        logger.Log(INFO, "Attempting to load Logical Screen Descriptor");

        //Load the LSD From GIF File 
        mInStream.read(reinterpret_cast<char*>(&mLSD), sizeof(byte) * sizeof(LSD));

        // Check to see if the GCT flag is set
        if (mLSD.Packed >> (int)LSDMask::GlobalColorTable) {
            logger.Log(INFO, "GCTD Presented - Loading GCTD");

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

            logger.Log(SUCCESS, "Loaded GCTD");
        } 

        logger.Log(SUCCESS, "Logical Screen Descriptor Initialized");
        return Status::success;
    }

    Status File::GenerateFrameMap()
    {
        logger.Log(INFO, "Generating Frame Map");
        
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
            logger.Log(INFO, "Loading Image Data");
            std::string rasterData = img.LoadData(this);

            prevPixelMap = pixelMap; 
            img.UpdatePixelMap(this, rasterData, pixelMap, prevPixelMap);
            mFrameMap.push_back(pixelMap);
            mImageData.push_back((void*)&img);
            byte nextByte = mInStream.peek();
            
            // Check if the file ended correctly (should end on 0x3B)
            if ((size_t)mInStream.tellg() == mFileSize - 1) {
                if (nextByte == TRAILER)
                    logger.Log(SUCCESS, "File ended naturally");
                else
                    logger.Log(WARNING, "File ended unaturally with byte [%02X]", nextByte);

                // There is nothing left to get from the file so close it
                mInStream.close();
                break;
            }
        }

        logger.Log(SUCCESS, "Generated Frame Map");
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
