#include "image.h"
#include "gif.h"
#include "common.h"
#include "gifmeta.h"
#include "logger.h"
#include "lzw.h"

namespace GIF 
{
    std::string Image::LoadData()
    {
        fread(&this->mDescriptor, sizeof(byte), sizeof(ImageDescriptor), this->mGIF->mFP);

        if ((this->mDescriptor.Packed >> (byte)ImgDescMask::LocalColorTable) & 0x1)
            LOG_INFO << "Located Local color table" << std::endl;
        else 
            LOG_INFO << "Local Color Table not found" << std::endl;

        // Load Image header into memory
        fread(&this->mDataHeader, sizeof(byte), sizeof(ImageDataHeader), this->mGIF->mFP);
        ReadDataSubBlocks();

        // Get raster data for the image frame by decompressing the data block from the gif
        std::string rasterData = "";
        if (this->mGIF->mLSD.packed >> (int)LSDMask::GlobalColorTable) {
            std::string rasterData = LZW::Decompress(this->mDataHeader, this->mGIF->mGCTD.colorCount, this->mData);
        } else {
            // TODO
            // Make a statement that works for Local Color tables too 
        }

        return rasterData;
    }

    void Image::ReadDataSubBlocks()
    {
        // TODO
        // while loop seems like it can be simplified for the first iteration
        // regarding the followSize
        
        byte nextByte = 0;
        byte followSize = this->mDataHeader.FollowSize;

        while (followSize--) {
            fread(&nextByte, sizeof(byte), 1, this->mGIF->mFP);
            this->mData.push_back(nextByte);
        }

        fread(&nextByte, sizeof(byte), 1, this->mGIF->mFP);

        while (true) {
            // Check for the end of a sub block
            if (!nextByte)
                break;

            followSize = nextByte;
            while (followSize--) {
                fread(&nextByte, sizeof(byte), 1, this->mGIF->mFP);
                this->mData.push_back(nextByte);
            }

            fread(&nextByte, sizeof(byte), 1, this->mGIF->mFP);
        }

        // TODO
        // Print compressed stream of data to file
    }

    void Image::CheckExtensions()
    {
        LOG_INFO << "Check for extensions" << std::endl; 
           
        // Allocate space in memory for a dummt extension header
        ExtensionHeader extensionCheck = {};

        // Continue to loop until the next byte is not an extension introducer
        while (true) {
            fread(&extensionCheck, sizeof(byte), sizeof(ExtensionHeader), this->mGIF->mFP);

            // If the dummy header contains an introducer for a extension, load the extension type
            if (extensionCheck.Introducer == EXTENSION_INTRODUCER) {
                LoadExtension(extensionCheck);
            } else {
                fseek(this->mGIF->mFP, -2, SEEK_CUR);
                return;
            }
        }
    }

    void Image::LoadExtension(const ExtensionHeader& headerCheck)
    {
        // If the header has a valid label for an extension, start each one
        // by loading the struct into memory and set its header to the one that was passed in
        fseek(this->mGIF->mFP, -2, SEEK_CUR);

        switch (headerCheck.Label) {
            case ExtensionLabel::PlainText:
            {
                LOG_INFO << "Loading plain text extension" << std::endl;

                // Load Header
                this->mExtensions.PlainText = {};
                fread(&this->mExtensions.PlainText.header, sizeof(byte), sizeof(ExtensionHeader), this->mGIF->mFP);

                // Load the block size into the struct and load the data of that size into the data buffer
                fread(&this->mExtensions.PlainText.blockSize, sizeof(byte), 1, this->mGIF->mFP);

                this->mExtensions.PlainText.data = new byte[this->mExtensions.PlainText.blockSize];
                fread(&this->mExtensions.PlainText.data, sizeof(byte), this->mExtensions.PlainText.blockSize, this->mGIF->mFP);

                LOG_INFO << "End of plain text extension" << std::endl;
                break;
            }
            case ExtensionLabel::GraphicsControl:
            {
                LOG_INFO << "Loading graphics control extension" << std::endl;

                // Load The entire Graphic Control Extension
                this->mExtensions.GraphicsControl = {};
                fread(&this->mExtensions.GraphicsControl, sizeof(byte), sizeof(GCE), this->mGIF->mFP);
                
                // Check for transparency
                if ((this->mExtensions.GraphicsControl.Packed >> (byte)GCEMask::TransparentColor) & 0x01) {
                    this->mTransparent = true;
                    this->mTransparentColorIndex = this->mExtensions.GraphicsControl.TransparentColorIndex;

                    LOG_INFO << "Transparent flag set in image" << std::endl;
                    LOG_INFO << "Tranparent Color Index: " << (int)this->mTransparentColorIndex << std::endl;
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
                this->mExtensions.Comment = {};
                fread(&this->mExtensions.Comment.Header, sizeof(byte), sizeof(ExtensionHeader), this->mGIF->mFP);

                // Read into the data section until a null terminator is hit
                byte nextByte = 0;
                for (int i = 0; nextByte != 0x00; i++) {
                    fread(&nextByte, sizeof(byte), 1, this->mGIF->mFP);
                    this->mExtensions.Comment.Data.push_back(nextByte);
                }

                LOG_INFO << "End of comment extension" << std::endl;
                break;
            }
            case ExtensionLabel::Application:
            {
                LOG_INFO << "Loading application extension" << std::endl;

                // Load Header
                this->mExtensions.Application = {};
                fread(&this->mExtensions.Application.header, sizeof(byte), sizeof(ExtensionHeader), this->mGIF->mFP);

                // Load the Block Length
                fread(&this->mExtensions.Application.blockLength, sizeof(byte), 1, this->mGIF->mFP);

                // Load Application Identifier
                fread(&this->mExtensions.Application.identifier, sizeof(byte), this->mExtensions.Application.blockLength, this->mGIF->mFP);
                
                // Load the authentication code
                byte tmp = 0;
                fread(&tmp, sizeof(byte), 1, this->mGIF->mFP);
                fread(&this->mExtensions.Application.authenticationCode, sizeof(byte), tmp, this->mGIF->mFP);

                // Check if the next byte in the file is the terminator
                fread(&tmp, sizeof(byte), 1, this->mGIF->mFP);
                if (!tmp)
                    LOG_INFO << "End of application extension" << std::endl;

                break;
            }
            default:
            {
                LOG_ERROR << "Recived invalid extension type [" << (int)headerCheck.Label <<  "]" << std::endl;
                fseek(this->mGIF->mFP, 2, SEEK_CUR); // Restore the file position to where it was after reading header
                break;
            }
        }
    }

    void Image::UpdatePixelMap(const std::string& rasterData, char** pixMap, UNUSED char** prevPixMap)
    {
        int disposalMethod = ((this->mExtensions.GraphicsControl.Packed >> (byte)GCEMask::Disposal) & 0x07);
        switch (disposalMethod) {
        case 0:
            break;
        case 1:
            DrawOverImage();
            //DrawOverImage(rasterData, pixMap, lsd);
            break;
        case 2:
            RestoreCanvasToBG();
            //RestoreCanvasToBG(pixMap, lsd);
            break;
        case 3:
            RestoreToPrevState();
            //RestoreToPrevState(pixMap, prevPixMap);
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            break;
        default:
            Debug::error(Severity::medium, "Image:", "undefined disposal method -", disposalMethod);
            break;
        }
    }

    void Image::DrawOverImage(const std::string& rasterData, char** pixelMap)
    {
        LOG_INFO << "Drawing over image" << std::endl;

        int currentChar = 0;
        for (int row = 0; row < this->mDescriptor.Height; row++) {
            for (int col = 0; col < this->mDescriptor.Width; col++) {
                if ((size_t)currentChar + 1 <= rasterData.size()) {
                    pixelMap[row][col] = rasterData.at(currentChar);
                    currentChar++;
                }
            }
        }
    }

    void Image::RestoreCanvasToBG(char** pixelMap)
    {
        LOG_INFO << "Restore canvas to background" << std::endl;
        std::unordered_map<int, std::string> codeTable = LZW::InitializeCodeTable(this->mGIF->mGCTD.colorCount);

        for (int row = 0; row < this->mDescriptor.Height; row++ ) {
            for (int col = 0; col < this->mDescriptor.Left; col++) {
                pixelMap[row][col] = codeTable[this->mGIF->mLSD.backgroundColorIndex][0];
            }
        } 
    }

    // TODO
    void Image::RestoreToPrevState(char** pixelMap, char** prevPixelMap)
    {
        LOG_INFO << "Restore canvas to previous state" << std::endl;
        
        //*pixMap = *prevPixMap;
    }
}
