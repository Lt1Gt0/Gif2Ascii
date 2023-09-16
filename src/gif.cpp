#include "gif.hpp"
#include "utils/error.hpp"
#include "utils/logger.hpp"
#include "utils/definitions.hpp"
#include "utils/types.hpp"
#include "gifmeta.hpp"
// #include "lzw.hpp"

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

        if (!mInStream)
            error(Severity::high, "GIF:", "Unable to open file: ", mPath.c_str());
        else 
            logger.Log(SUCCESS, "Opened [%s]", mPath.c_str());

        CalculateFileSize();
        logger.Log(DEBUG, "Total file size: %dkB (%db)", mFileSize / 1024, mFileSize);

        Read();

        mInStream.close();
        logger.Log(SUCCESS, "Buffered GIF Data");
    }

    File::~File() {}

    void File::CalculateFileSize()
    {
        // Get beginning and end of file stream
        const auto begin = mInStream.tellg();
        mInStream.seekg(0, std::ios::end);
        const auto end = mInStream.tellg();

        // Set file size and reset position
        mFileSize = end - begin;
        mInStream.seekg(std::ios::beg);
    }

    void File::Read()
    {
        logger.Log(INFO, "Reading GIF Information");

        ParseHeader();
        ParseLSD();
        GenerateFrameMap();

        logger.Log(SUCCESS, "Read GIF Information");
    }


    /*
     * TODO:
     * Add more return statuses for better error handling
     *
     * RETURNS
     *
     * 0 -> Success
     * 1 -> Issue
     */
    int File::Read(void* buf, size_t size, bool changePos, size_t offset)
    {
        int status = 0;

        try {
            if (changePos)
                mInStream.seekg(offset);
            else
                offset = mInStream.cur;
        } catch (std::exception err) {
            logger.Log(ERROR, err.what());
            logger.Log(ERROR, "Attempted to update file pos to: %d", offset);
            error(Severity::high, "File Pos Update:", "Error Setting position of file Read");
        }

        try {
            mInStream.read(reinterpret_cast<char*>(buf), size);
        } catch (std::exception err) {
            logger.Log(ERROR, err.what());
            logger.Log(ERROR, "Attempted to cast");
            error(Severity::high, "Cast Error:", "Error casting buffer type to char");
        }

        return status;
    }

    void File::ParseHeader()
    {
        if (!mInStream)
            error(Severity::high, "File Buffer:", "Buffer was never initialized or was closed");

        // Load the GIF header into memory
        Read(&mDS.header, sizeof(DataStream::Header));

        if (!ValidHeader())
            error(Severity::high, "Invalid File Header");

        logger.Log(SUCCESS, "Parsed GIF Header");
    }

    bool File::ValidHeader()
    {
        // Check if a valid version and signature is provided
        if (!(std::strncmp(mDS.header.signature, GIF_SIGNATURE, 3)) && 
            !(strncmp(mDS.header.version, GIF_87A, 3) || strncmp(mDS.header.version, GIF_89A, 3))) {
            logger.Log(ERROR, "Header: Invalid Header information");
            return false;
        }

        // Check if the version is supported by my decoder
        if (std::strncmp(mDS.header.version, SUPPORTED_VERSIONS[0], 3)) {
            logger.Log(ERROR, "Version Issue: Gif version is not supported by this decoder", mDS.header.version);
            return false;
        }

        return true;
    }


    void File::ParseLSD()
    {
        logger.Log(INFO, "Attempting to load Logical Screen Descriptor");

        //Load the LSD From GIF File 
        Read(&mDS.lsd, sizeof(LogicalScreen::LSD));

        // Check to see if the GCT flag is set
        if (mDS.lsd.packed >> (int)LogicalScreen::Mask::GlobalColorTable) {
            logger.Log(INFO, "GCTD Presented - Loading GCTD");
            ParseGCTD();
        } 

        logger.Log(SUCCESS, "Logical Screen Descriptor Initialized");
    }

    void File::ParseGCTD()
    {
        // mGCTD = {};
        mDS.gctd.sizeInLSD = (mDS.lsd.packed >> (byte)LogicalScreen::Mask::Size) & 0x07;
        mDS.gctd.colorCount = pow(2, mDS.gctd.sizeInLSD + 1);
        mDS.gctd.byteLength = 3 * mDS.gctd.colorCount;

        // Generate the GCT from each color present in file
        mGCT = new Color[mDS.gctd.colorCount];
        for (int i = 0; i < mDS.gctd.colorCount; i++) {
            Color color = {};
            Read(&color, sizeof(Color));
            mGCT[i] = color; 
        }

        logger.Log(SUCCESS, "Loaded GCTD");
    }

    void File::GenerateFrameMap()
    {
        logger.Log(INFO, "Generating Frame Map");
        
        // The pixel map will be initialized as a single vector
        // to mimic a two dimensional array, elements are accessed like so
        // (char) pixel = PixelMap.at(ROW * width) + COL
        Vec<char> pixelMap(mDS.lsd.width * mDS.lsd.height, 0);
        Vec<char> prevPixelMap = pixelMap;

        // Build up each frame for the gif
        while (true) {
            Image img = Image();
            img.mData = new Data::Data;

            // Load Image Extenstion information before proceeding with parsing image data
            img.CheckExtensions(this);

            // Load the decompressed image data and draw the frame
            logger.Log(INFO, "Loading Image Data");
            std::string rasterData = img.LoadData(this);

            prevPixelMap = pixelMap; 
            img.UpdatePixelMap(this, rasterData, pixelMap, prevPixelMap);
            mFrameMap.push_back(pixelMap);
            mDS.data.push_back(*img.mData);
            byte nextByte = mInStream.peek();

            // Check if the file ended correctly (should end on 0x3B)
            if ((size_t)mInStream.tellg() == mFileSize - 1) {
                if (nextByte == TRAILER)
                    logger.Log(SUCCESS, "File ended naturally");
                else
                    logger.Log(WARNING, "File ended unaturally with byte [%02X]", nextByte);

                // There is nothing left to get from the file so close it
                mInStream.close();
            }
            break;
        }

        logger.Log(SUCCESS, "Generated Frame Map");
    }

    void File::DumpInfo(std::string dumpPath)
    {
        std::ofstream dump(dumpPath);
        
        if (!dump.is_open())
            error(Severity::medium, "GIF:", "Unable to open dump file");
        
        // Header information
        dump << "File: " << mPath << std::endl;
        dump << "Version: " << mDS.header.version[0] << mDS.header.version[1] << mDS.header.version[2] << std::endl;

        // Logical Screen Descriptor
        dump << "LSD Width: " << (int)mDS.lsd.width << std::endl; 
        dump << "LSD Height: " << (int)mDS.lsd.height << std::endl; 
        dump << "GCTD Present: " << (int)((mDS.lsd.packed >> (byte)LogicalScreen::Mask::GlobalColorTable) & 0x1) << std::endl;
        dump << "LSD Background Color Index: " << (int)mDS.lsd.backgroundColorIndex << std::endl; 
        dump << "LSD Pixel Aspect Ratio: " << (int)mDS.lsd.pixelAspectRatio << std::endl; 

        // GCTD / GCT
        if ((mDS.lsd.packed >> (byte)LogicalScreen::Mask::GlobalColorTable) & 0x1) {
            dump << "GCT Size: " << (int)mDS.gctd.sizeInLSD << std::endl;
            dump << "GCT Color Count: " << (int)mDS.gctd.colorCount << std::endl;
            dump << "GCT Size in bytes: " << (int)mDS.gctd.byteLength << std::endl;
            
            dump.setf(std::ios::hex, std::ios::basefield);    
            for (int i = 0; i < mDS.gctd.colorCount; i++) {
                    dump << "\tRed: " << std::uppercase << (int)mGCT[i].red << std::endl;
                    dump << "\tGreen: " << std::uppercase << (int)mGCT[i].green << std::endl;
                    dump << "\tBlue: " << std::uppercase << (int)mGCT[i].blue << std::endl;
                    dump << std::endl;
            } 
            dump.unsetf(std::ios::hex);
        }
            
        dump.close();
    }
}
