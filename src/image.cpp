#include "gif.hpp"
#include "gifmeta.hpp"

#include <cstdint>
#include <stdio.h>
#include <unordered_map>
#include "lzw.hpp"
#include "utils/logger.hpp"
#include "utils/error.hpp"

namespace GIF
{
    Image::Image() {}
    Image::~Image() {}

    std::string Image::LoadData(File* const gif)
    {
        // Load the Image Descriptor into memory
        gif->Read(&mData->graphic.imgDesc, sizeof(Data::Graphic::ImageDescriptor));

        // TODO - Add support for LCT in GIFS that require it
        if ((mData->graphic.imgDesc.packed >> (byte)Data::Graphic::ImgDescMask::LocalColorTable) & 0x1)
            logger.Log(INFO, "Loading Local Color Table"); 
        else
            logger.Log(INFO, "Local Color Table flag not set"); 
      
        // Load the image header into memory
        gif->Read(&mData->graphic.data.lzwMinimum, 1);
        gif->Read(&mData->graphic.data.block.size, 1);

        ReadDataSubBlock(gif);

        // Get the raster data from the image frame by decompressing the data block from the gif
        return LZW::Decompress(mData->graphic.data, mColorTableSize);
    }

    void Image::ReadDataSubBlock(File* const gif)
    {
        // TODO 
        // while loop seems like it can be simplified for the first iteration
        // regarding the followSize
        
        logger.Log(INFO, "Loading Data sub blocks"); 
        
        byte nextByte = 0;
        int followSize = mData->graphic.data.block.size;

        while (followSize--) {
            nextByte = gif->mInStream.get();
            mData->graphic.data.block.data.push_back(nextByte);
        }

        nextByte = gif->mInStream.get();

        while (true) {
            // Check for the end of sub block
            if (!nextByte)
                break;
            
            followSize = nextByte;
            while (followSize--) {
                nextByte = gif->mInStream.get();
                mData->graphic.data.block.data.push_back(nextByte);
            }
        }

        // TODO
        // Print out the compressed stream of data 
        logger.Log(SUCCESS, "Loaded Data sub blocks"); 
    }

    void Image::CheckExtensions(File* const gif)
    {
        logger.Log(INFO, "Checking for extensions"); 

        // Allocate space in memory for an extension header
        Data::ExtensionHeader extensionCheck = {};
        
        // Continue to loop until the next byte is not an extension introducer
        while (true) {
            gif->Read(&extensionCheck, sizeof(Data::ExtensionHeader));

            // If the dummy header contains an introducer for a extension, load the extension type
            
            // Restore filepos to where it was before the extension check was loaded
            gif->mInStream.seekg((size_t)gif->mInStream.tellg() - 2);
            if (extensionCheck.introducer == EXTENSION_INTRODUCER)
                LoadExtension(gif, extensionCheck);
            else
                return;
        }
    }

    void Image::LoadExtension(File* const gif, const Data::ExtensionHeader& headerCheck)
    {
        switch (headerCheck.label) {
            case Data::ExtensionLabel::PlainText:
            {
                logger.Log(INFO, "Loading plain text extension"); 

                // Load Header
                mData->graphic.pte = {}; 
                gif->Read(&mData->graphic.pte, sizeof(Data::Graphic::PlainTextExtension));

                // Load the block size into the struct and load the data of that size into the data buffer
                mData->graphic.pte.blockSize = gif->mInStream.get();

                mData->graphic.pte.data = new byte[mData->graphic.pte.blockSize];
                gif->Read(&mData->graphic.pte.data, mData->graphic.pte.blockSize);

                logger.Log(INFO, "End of plain text extension"); 
                break;
            }
            case Data::ExtensionLabel::GraphicsControl:
            {
                logger.Log(INFO, "Loading graphics control extension"); 

                // Load The entire Graphic Control Extension
                mData->graphic.gce = {};
                gif->Read(&mData->graphic.gce, sizeof(Data::Graphic::GCE));
                
                // Check for transparency
                if ((mData->graphic.gce.packed >> (byte)Data::Graphic::GCEMask::TransparentColor) & 0x01) {
                    mTransparent = true;
                    mTransparentColorIndex = mData->graphic.gce.transparentColorIndex;

                    logger.Log(INFO, "Transparent flag set in image"); 
                    logger.Log(INFO, "Tranparent Color Index: %d", (int)mTransparentColorIndex);
                } else {
                    logger.Log(INFO, "Transparent flag not set" ); 
                }
                
                logger.Log(INFO, "End of graphics control extension"); 
                break;
            }
            case Data::ExtensionLabel::Comment:
            {
                logger.Log(INFO, "Loading comment extension"); 

                // Load Header
                mData->special.comment = {};
                gif->Read(&mData->special.comment.header, sizeof(Data::ExtensionHeader));

                // Read into the data section until a null terminator is hit
                byte nextByte = 0;
                while (nextByte != 0x00) {
                    nextByte = gif->mInStream.get();
                    mData->special.comment.data.push_back(nextByte);
                }

                logger.Log(INFO, "End of comment extension"); 
                break;
            }
            case Data::ExtensionLabel::Application:
            {
                logger.Log(INFO, "Loading application extension"); 

                // Load Header
                mData->special.application = {};
                gif->Read(&mData->special.application.header, sizeof(Data::ExtensionHeader));

                // Load the Block Length
                mData->special.application.blockLength = gif->mInStream.get();

                // Load Application Identifier
                mData->special.application.identifier = new byte[mData->special.application.blockLength];
                gif->Read(&mData->special.application.identifier, mData->special.application.blockLength);
                
                // Load the authentication code
                byte tmp = 0;
                tmp = gif->mInStream.get();
                mData->special.application.authenticationCode = new byte[tmp];
                gif->Read(&mData->special.application.authenticationCode, tmp);

                // Check if the next byte in the file is the terminator
                tmp = gif->mInStream.get();
                if (!tmp)
                    logger.Log(INFO, "End of application extension"); 

                break;
            }
            default:
            {
                logger.Log(ERROR, "Recived invalid extension type [%d]", (int)headerCheck.label);
                gif->mInStream.seekg((size_t)gif->mInStream.tellg() - 2); // Restore the file position to where it was after reading header
                break;
            }
        }
    }

    void Image::UpdatePixelMap(UNUSED File* const gif, UNUSED const std::string& rasterData, std::vector<char> pixMap, UNUSED std::vector<char> prevPixMap)
    {
        // Because each gif can have a different disposal method for different frames (according to GIF89a)
        // it is best to handle each disposal method instread of printing the decompressed codestream directly
        int disposalMethod = ((mData->graphic.gce.packed >> (byte)Data::Graphic::GCEMask::Disposal) & 0x07);
        switch (disposalMethod) {
            case 0:
                break;
            case 1:
                DrawOverImage(gif, rasterData, pixMap);
                break;
            case 2:
                RestoreCanvasToBG(gif, pixMap);
                break;
            case 3:
                RestoreToPrevState(pixMap, prevPixMap);
                break;
            case 4:
            case 5:
            case 6:
            case 7:
                break;
            default:
                error(Severity::medium, "Image:", "undefined disposal method -", disposalMethod);
                break;
        }
    }

    void Image::DrawOverImage(File* const gif, const std::string& rasterData, std::vector<char> pixelMap)
    {
        logger.Log(INFO, "Drawing over image"); 
        int offset = 0;
        int currentChar = 0;
        for (int row = 0; row < mData->graphic.imgDesc.height; row++) {
            for (int col = 0; col < mData->graphic.imgDesc.width; col++) {
                if ((size_t)currentChar + 1 <= rasterData.size()) {
                    offset = ((row + mData->graphic.imgDesc.top) * gif->mDS.lsd.width) + (col + mData->graphic.imgDesc.left);
                    pixelMap.at(offset) = rasterData.at(currentChar);
                    currentChar++;
                }
            }
        }
    }

    void Image::RestoreCanvasToBG(File* const gif, std::vector<char> pixelMap)
    {
        logger.Log(INFO, "Restore canvas to background"); 
        std::unordered_map<int, std::string> codeTable = LZW::InitializeCodeTable(mColorTableSize);

        int offset = 0;
        for (int row = 0; row < mData->graphic.imgDesc.height; row++ ) {
            for (int col = 0; col < mData->graphic.imgDesc.left; col++) {
                offset = ((row + mData->graphic.imgDesc.top) * gif->mDS.lsd.width) + (col + mData->graphic.imgDesc.left);
                pixelMap.at(offset) = codeTable[gif->mDS.lsd.backgroundColorIndex][0]; 
            }
        } 
    }

    void Image::RestoreToPrevState(std::vector<char> pixMap, std::vector<char> prevPixMap)
    {
        logger.Log(INFO, "Restore canvas to previous state"); 
        pixMap = prevPixMap;
    }

    void Image::PrintDescriptor()
    {
        using namespace Data::Graphic;
        logger.Log(DEBUG, "------- Image Descriptor -------");
        logger.Log(DEBUG, "Seperator: %X", mData->graphic.imgDesc.seperator);
        logger.Log(DEBUG, "Image Left: %d", mData->graphic.imgDesc.left);
        logger.Log(DEBUG, "Image Top: %d", mData->graphic.imgDesc.top);
        logger.Log(DEBUG, "Image Width: %d", mData->graphic.imgDesc.width);
        logger.Log(DEBUG, "Image Height: %d", mData->graphic.imgDesc.height);
        logger.Log(DEBUG, "Local Color Table Flag: %d", (mData->graphic.imgDesc.packed >> (byte)ImgDescMask::LocalColorTable) & 0x1);
        logger.Log(DEBUG, "Interlace Flag: %d", (mData->graphic.imgDesc.packed >> (byte)ImgDescMask::Interlace) & 0x1);
        logger.Log(DEBUG, "Sort Flag: %d", (mData->graphic.imgDesc.packed >> (byte)ImgDescMask::IMGSort) & 0x1);
        logger.Log(DEBUG, "Size of Local Color Table: %d", (mData->graphic.imgDesc.packed >> (byte)ImgDescMask::IMGSize) & 0x7);
        logger.Log(DEBUG, "--------------------------------");
    }

    void Image::PrintData()
    {
        logger.Log(DEBUG, "\n------- Image Data -------");
        logger.Log(DEBUG, "LZW Minimum: 0x%X", mData->graphic.data.lzwMinimum);
        logger.Log(DEBUG, "Initial Follow Size: 0x%X", mData->graphic.data.block.size);
        logger.Log(DEBUG, "--------------------------");
    }

    void Image::PrintSubBlockData(std::vector<byte>* block)
    {
        logger.Log(DEBUG, "\n------- Block Data -------");
        logger.Log(DEBUG, "Size: %ld\n", block->size());
        for (int i = 0; i < (int)block->size(); i++) {
            fprintf(stdout, "%X ", block->at(i));
        }
        logger.Log(DEBUG, "\n--------------------------");
    }

    void Image::DumpInfo(std::string dumpPath)
    {
        std::ofstream dump (dumpPath);
        
        if (!dump.is_open())
            error(Severity::medium, "GIF:", "Unable to open dump file");
        
        // Header information
        // dump << "File: " << mPath << std::endl;
        // dump << "Version: " << mHeader.Version[0] << mHeader.Version[1] << mHeader.Version[2] << std::endl;
        
        // Logical Screen Descriptor
        // dump << "LSD Width: " << (int)mLSD.Width << std::endl; 
        // dump << "LSD Height: " << (int)mLSD.Height << std::endl; 
        // dump << "GCTD Present: " << (int)((mLSD.Packed >> (byte)LSDMask::GlobalColorTable) & 0x1) << std::endl;
        // dump << "LSD Background Color Index: " << (int)mLSD.BackgroundColorIndex << std::endl; 
        // dump << "LSD Pixel Aspect Ratio: " << (int)mLSD.PixelAspectRatio << std::endl; 
        // 
        // // GCTD / GCT
        // if ((mLSD.Packed >> (byte)LSDMask::GlobalColorTable) & 0x1) {
        //     dump << "GCT Size: " << (int)mGCTD.SizeInLSD << std::endl;
        //     dump << "GCT Color Count: " << (int)mGCTD.ColorCount << std::endl;
        //     dump << "GCT Size in bytes: " << (int)mGCTD.ByteLength << std::endl;
        //     
        //     dump.setf(std::ios::hex, std::ios::basefield);    
        //     for (int i = 0; i < mGCTD.ColorCount; i++) {
        //             dump << "\tRed: " << std::uppercase << (int)mGCT[i].Red << std::endl;
        //             dump << "\tGreen: " << std::uppercase << (int)mGCT[i].Green << std::endl;
        //             dump << "\tBlue: " << std::uppercase << (int)mGCT[i].Blue << std::endl;
        //             dump << std::endl;
        //     } 
        //     dump.unsetf(std::ios::hex);
        // }
         
        dump.close();
    }
}
