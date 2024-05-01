#pragma once
#ifndef _LZW_HPP
#define _LZW_HPP

#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <string>

#include "image.hpp"
#include "gifmeta.hpp"

#define SPECIAL_CODE_COUNT 2

namespace LZW 
{
    using namespace std;
    string Decompress(const ImageDataHeader& imgHeader, const uint8_t colorTableSize, vector<uint8_t> codestream);

    /**
     * Initialize a code table based off the size of a given color table
     * 
     * @param colorTableSize
     * @return std::unordered_map<int, std::string> Code Table
     */
    unordered_map<int, string> InitializeCodeTable(const uint8_t colorTableSize);
}

#endif // _LZW_HPP
