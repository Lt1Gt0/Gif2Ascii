#include "gif.h"
#include "gifmeta.h"

#include <cstdint>
#include <stdio.h>
#include <unordered_map>
#include "lzw.h"
#include "logger.h"

namespace GIF
{
    Image::Image() {}
    Image::~Image() {}

    std::string Image::LoadData(File* const gif)
    {
        // Load the Image Descriptor into memory
        fread(&mDescriptor, sizeof(byte), sizeof(ImageDescriptor), gif->mFP);

        // TODO - Add support for LCT in GIFS that require it
        if ((mDescriptor.Packed >> (byte)ImgDescMask::LocalColorTable) & 0x1)
            LOG_INFO << "Loading Local Color Table" << std::endl;
        else
            LOG_INFO << "Local Color Table flag not set" << std::endl;

        // Load the image header into memory
        fread(&mDataHeader, sizeof(byte), sizeof(ImageDataHeader), gif->mFP); // Only read 2 bytes of file steam for LZW min and Follow Size 
        ReadDataSubBlocks(gif);

        // Get the raster data from the image frame by decompressing the data block from the gif
        std::string rasterData = LZW::Decompress(mDataHeader, mColorTableSize, mData);
        return rasterData;
    }

    void Image::ReadDataSubBlocks(File* const gif)
    {
        // TODO 
        // while loop seems like it can be simplified for the first iteration
        // regarding the followSize
        
        byte nextByte = 0;
        int followSize = mDataHeader.followSize;
        
        while (followSize--) {
            fread(&nextByte, sizeof(byte), 1, gif->mFP);
            mData.push_back(nextByte);
        }

        fread(&nextByte, sizeof(byte), 1, gif->mFP);

        while (true) {
            // Check for the end of sub block
            if (!nextByte)
                break;
            
            followSize = nextByte;
            while (followSize--) {
                fread(&nextByte, sizeof(byte), 1, gif->mFP);
                mData.push_back(nextByte);
            }

            fread(&nextByte, sizeof(byte), 1, gif->mFP);
        }

        // TODO
        // Print out the compressed stream of data 
    }

    void Image::CheckExtensions(File* const gif)
    {
        LOG_INFO << "Checking for extensions" << std::endl;

        // Allocate space in memory for an extension header
        ExtensionHeader extensionCheck = {};

        // Continue to loop until the next byte is not an extension introducer
        while (true) {
            fread(&extensionCheck, sizeof(byte), sizeof(ExtensionHeader), gif->mFP);

            // If the dummy header contains an introducer for a extension, load the extension type
            if (extensionCheck.introducer == EXTENSION_INTRODUCER) {
                LoadExtension(gif, extensionCheck);
            } else {
                fseek(gif->mFP, -2, SEEK_CUR);
                return;
            }
        }
    }

    void Image::LoadExtension(File* const gif, const ExtensionHeader& headerCheck)
    {
        // If the header has a valid label for an extension, start each one
        // by loading the struct into memory and set its header to the one that was passed in
        fseek(gif->mFP, -2, SEEK_CUR);

        switch (headerCheck.label) {
            case ExtensionLabel::PlainText:
            {
                LOG_INFO << "Loading plain text extension" << std::endl;

                // Load Header
                mExtensions.plainText = {};
                fread(&mExtensions.plainText.header, sizeof(byte), sizeof(ExtensionHeader), gif->mFP);

                // Load the block size into the struct and load the data of that size into the data buffer
                fread(&mExtensions.plainText.blockSize, sizeof(byte), 1, gif->mFP);

                mExtensions.plainText.data = new byte[mExtensions.plainText.blockSize];
                fread(&mExtensions.plainText.data, sizeof(byte), mExtensions.plainText.blockSize, gif->mFP);

                LOG_INFO << "End of plain text extension" << std::endl;
                break;
            }
            case ExtensionLabel::GraphicsControl:
            {
                LOG_INFO << "Loading graphics control extension" << std::endl;

                // Load The entire Graphic Control Extension
                mExtensions.graphicsControl = {};
                fread(&mExtensions.graphicsControl, sizeof(byte), sizeof(GCE), gif->mFP);
                
                // Check for transparency
                if ((mExtensions.graphicsControl.packed >> (byte)GCEMask::TransparentColor) & 0x01) {
                    mTransparent = true;
                    mTransparentColorIndex = mExtensions.graphicsControl.transparentColorIndex;

                    LOG_INFO << "Transparent flag set in image" << std::endl;
                    LOG_INFO << "Tranparent Color Index: " << (int)mTransparentColorIndex << std::endl;
                } else {
                    LOG_INFO << "Transparent flag not set" << std::endl; 
                }
                
                LOG_INFO << "End of graphics control extension" << std::endl;
                break;
            }
            case ExtensionLabel::Comment:
            {
                LOG_INFO << "Loading comment extension" << std::endl;

                // Load Header
                mExtensions.comment = {};
                fread(&mExtensions.comment.header, sizeof(byte), sizeof(ExtensionHeader), gif->mFP);

                // Read into the data section until a null terminator is hit
                byte nextByte = 0;
                for (int i = 0; nextByte != 0x00; i++) {
                    fread(&nextByte, sizeof(byte), 1, gif->mFP);
                    mExtensions.comment.data.push_back(nextByte);
                }

                LOG_INFO << "End of comment extension" << std::endl;
                break;
            }
            case ExtensionLabel::Application:
            {
                LOG_INFO << "Loading application extension" << std::endl;

                // Load Header
                mExtensions.application = {};
                fread(&mExtensions.application.header, sizeof(byte), sizeof(ExtensionHeader), gif->mFP);

                // Load the Block Length
                fread(&mExtensions.application.blockLength, sizeof(byte), 1, gif->mFP);

                // Load Application Identifier
                fread(&mExtensions.application.identifier, sizeof(byte), mExtensions.application.blockLength, gif->mFP);
                
                // Load the authentication code
                byte tmp = 0;
                fread(&tmp, sizeof(byte), 1, gif->mFP);
                fread(&mExtensions.application.authenticationCode, sizeof(byte), tmp, gif->mFP);

                // Check if the next byte in the file is the terminator
                fread(&tmp, sizeof(byte), 1, gif->mFP);
                if (!tmp)
                    LOG_INFO << "End of application extension" << std::endl;

                break;
            }
            default:
            {
                LOG_ERROR << "Recived invalid extension type [" << (int)headerCheck.label <<  "]" << std::endl;
                fseek(gif->mFP, 2, SEEK_CUR); // Restore the file position to where it was after reading header
                break;
            }
        }
    }

    void Image::UpdatePixelMap(UNUSED File* const gif, UNUSED const std::string& rasterData, std::vector<char> pixMap, UNUSED std::vector<char> prevPixMap)
    {
        // Because each gif can have a different disposal method for different frames (according to GIF89a)
        // it is best to handle each disposal method instread of printing the decompressed codestream directly
        int disposalMethod = ((mExtensions.graphicsControl.packed >> (byte)GCEMask::Disposal) & 0x07);
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
        LOG_INFO << "Drawing over image" << std::endl;
        int offset = 0;
        int currentChar = 0;
        for (int row = 0; row < mDescriptor.Height; row++) {
            for (int col = 0; col < mDescriptor.Width; col++) {
                if ((size_t)currentChar + 1 <= rasterData.size()) {
                    offset = ((row + mDescriptor.Top) * gif->mLSD.width) + (col + mDescriptor.Left);
                    pixelMap.at(offset) = rasterData.at(currentChar);
                    currentChar++;
                }
            }
        }
    }

    void Image::RestoreCanvasToBG(File* const gif, std::vector<char> pixelMap)
    {
        LOG_INFO << "Restore canvas to background" << std::endl;
        std::unordered_map<int, std::string> codeTable = LZW::InitializeCodeTable(mColorTableSize);

        int offset = 0;
        for (int row = 0; row < mDescriptor.Height; row++ ) {
            for (int col = 0; col < mDescriptor.Left; col++) {
                offset = ((row + mDescriptor.Top) * gif->mLSD.width) + (col + mDescriptor.Left);
                pixelMap.at(offset) = codeTable[gif->mLSD.backgroundColorIndex][0]; 
            }
        } 
    }

    void Image::RestoreToPrevState(std::vector<char> pixMap, std::vector<char> prevPixMap)
    {
        LOG_INFO << "Restore canvas to previous state" << std::endl;
        pixMap = prevPixMap;
    }

    /*
    void Image::PrintDescriptor()
    {
        Debug::Print("------- Image Descriptor -------");
        Debug::Print("Seperator: %X", mDescriptor.Seperator);
        Debug::Print("Image Left: %d", mDescriptor.Left);
        Debug::Print("Image Top: %d", mDescriptor.Top);
        Debug::Print("Image Width: %d", mDescriptor.Width);
        Debug::Print("Image Height: %d", mDescriptor.Height);
        Debug::Print("Local Color Table Flag: %d", (mDescriptor.Packed >> (byte)ImgDescMask::LocalColorTable) & 0x1);
        Debug::Print("Interlace Flag: %d", (mDescriptor.Packed >> (byte)ImgDescMask::Interlace) & 0x1);
        Debug::Print("Sort Flag: %d", (mDescriptor.Packed >> (byte)ImgDescMask::IMGSort) & 0x1);
        Debug::Print("Size of Local Color Table: %d", (mDescriptor.Packed >> (byte)ImgDescMask::IMGSize) & 0x7);
        Debug::Print("--------------------------------");
    }

    void Image::PrintData()
    {
        Debug::Print("\n------- Image Data -------");
        Debug::Print("LZW Minimum: 0x%X", mHeader.LZWMinimum);
        Debug::Print("Initial Follow Size: 0x%X", mHeader.FollowSize);
        Debug::Print("--------------------------");
    }

    void Image::PrintSubBlockData(std::vector<byte>* block)
    {
        Debug::Print("\n------- Block Data -------");
        Debug::Print("Size: %ld\n", block->size());
        for (int i = 0; i < (int)block->size(); i++) {
            fprintf(stdout, "%X ", block->at(i));
        }
        Debug::Print("\n--------------------------");
    }
    */
}
