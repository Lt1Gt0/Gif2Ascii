#pragma once
#ifndef _LZW_H
#define _LZW_H

#include <vector>
#include <unordered_map>
#include <string>

#include "common.h"
#include "gif.h"
#include "gifmeta.h"

namespace LZW 
{
    constexpr byte SPECIAL_CODE_COUNT {2};

    std::string Decompress(const GIF::ImageDataHeader& imgHeader, const byte colorTableSize, std::vector<byte> codestream);

    /**
     * Initialize a code table based off the size of a given color table
     * 
     * @param colorTableSize
     * @return std::unordered_map<int, std::string> Code Table
     */
    std::unordered_map<int, std::string> InitializeCodeTable(const byte colorTableSize);
}

#endif // _LZW_H
