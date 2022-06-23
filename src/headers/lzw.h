#pragma once
#ifndef _LZW_H
#define _LZW_H

#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <string>

#include "image.h"

namespace LZW 
{
    std::string Decompress(ImageDataHeader* imgHeader, std::vector<std::vector<uint8_t>>* colorTable, std::vector<uint8_t> codestream);

    /**
     * Initialize a code table based off the size of a given color table
     * 
     * (For the record, this is not the best way to initialize a code table for
     * LZW decompression, it starts at 'A' and goes beyond that. This all ultimatly
     * means it will be short by 65 codes if there is any code table that is intialized
     * past 191 codes)
     * 
     * @param colorTable 
     * @return std::unordered_map<int, std::string> Code Table
     */
    std::unordered_map<int, std::string> InitializeCodeTable(std::vector<std::vector<uint8_t>>* colorTable);
}

#endif // _LZW_H