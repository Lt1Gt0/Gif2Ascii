#include "imagedata.h"

#include <iostream>

namespace ImageData
{
    Image::Image()
    {

    }

    void Image::ReadDataSubBlocks(FILE* file)
    {
        std::vector<uint8_t> blockData;
        blockData.resize(header->FollowSize);
        for (int i = 0; i < (int)blockData.size(); i++) {
            fread(&blockData[i], 1, 1, file);
        }

        printf("Finished Reading first sub block\n");
        subBlocks.push_back(blockData);

        uint8_t* nextByte = (uint8_t*)malloc(sizeof(uint8_t));
        bool readingSubBlocks = true;

        while (readingSubBlocks) {
            fread(nextByte, 1, 1, file);
            printf("Next Follow Size: 0x%X\n", *nextByte);

            if (*nextByte != 0x00) {
                printf("Appending new sub block");
                subBlocks.push_back(blockData);
                PrintSubBlockData(blockData); // Debug

                blockData.clear();
                blockData.resize(*nextByte);

                for (int i = 0; i < (int)blockData.size(); i++) {
                    fread(&blockData[i], 1, 1, file);
                }
            } else {
                printf("Finished Reading Data Sub Block\n");
                // fread(nextByte, 1, 1, file); // Read one more byte to pass null terminator
                readingSubBlocks = false;
            }
        }
    }
    
    void Image::PrintDescriptor()
    {
        printf("------- Image Descriptor -------\n");
        printf("Seperator: %X\n", descriptor->Seperator);
        printf("Image Left: %d\n", descriptor->Left);
        printf("Image Right: %d\n", descriptor->Right);
        printf("Image Width: %d\n", descriptor->Width);
        printf("Image Height: %d\n", descriptor->Height);
        printf("Local Color Table Flag: %d\n", (descriptor->Packed >> ImgDescMask::LocalColorTable) & 0x1);
        printf("Interlace Flag: %d\n", (descriptor->Packed >> ImgDescMask::Interlace) & 0x1);
        printf("Sort Flag: %d\n", (descriptor->Packed >> ImgDescMask::IMGSort) & 0x1);
        printf("Size of Local Color Table: %d\n", (descriptor->Packed >> ImgDescMask::IMGSize) & 0x7);
        printf("--------------------------------\n");
    }

    void Image::PrintData()
    {
        printf("\n------- Image Data -------\n");
        printf("LZW Minimum: 0x%X\n", header->LZWMinimum);
        printf("Initial Follow Size: 0x%X\n", header->FollowSize);
        printf("--------------------------\n");
    }

    void Image::PrintSubBlockData(std::vector<uint8_t> block)
    {
        printf("\n------- Sub Block -------\n");
        printf("Size: %ld\n", block.size());
        for (int i = 0; i < (int)block.size(); i++) {
            printf("%X ", block.at(i));
        }
        printf("\n-------------------------\n");
    }
}