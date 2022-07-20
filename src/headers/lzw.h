#pragma once
#ifndef _LZW_H
#define _LZW_H

#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <string>

#include "image.h"
#include "gifmeta.h"

#define SPECIAL_CODE_COUNT 2

namespace LZW 
{
    using namespace std;
    string Decompress(ImageDataHeader* imgHeader, uint8_t colorTableSize, vector<uint8_t> codestream);

    /**
     * Initialize a code table based off the size of a given color table
     * 
     * @param colorTableSize
     * @return std::unordered_map<int, std::string> Code Table
     */
    unordered_map<int, string> InitializeCodeTable(uint8_t colorTableSize);
}

#endif // _LZW_H
