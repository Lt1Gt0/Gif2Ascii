#pragma once
#ifndef _LZW_HPP_
#define _LZW_HPP_

#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <string>

#include "gifmeta.hpp"

namespace LZW 
{
    constexpr int SPECIAL_CODE_COUNT {2};

    std::string Decompress(const GIF::ImageDataHeader& imgHeader, const byte colorTableSize, std::vector<byte> codestream);

    /**
     * Initialize a code table based off the size of a given color table
     * 
     * @param colorTableSize
     * @return std::unordered_map<int, std::string> Code Table
     */
    std::unordered_map<int, std::string> InitializeCodeTable(const byte colorTableSize);
}

#endif // _LZW_HPP_
