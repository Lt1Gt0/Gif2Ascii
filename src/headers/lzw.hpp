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
    using namespace GIF::Data::Graphic;
    constexpr int SPECIAL_CODE_COUNT {2};

    std::string Decompress(const ImageData& ImgData, const byte colorTableSize);

    /**
     * Initialize a code table based off the size of a given color table
     * 
     * @param colorTableSize
     * @return std::unordered_map<int, std::string> Code Table
     */
    std::unordered_map<int, std::string> InitializeCodeTable(const byte colorTableSize);
}

#endif // _LZW_HPP_
