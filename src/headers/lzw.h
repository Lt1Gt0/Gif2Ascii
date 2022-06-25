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
    using namespace std;
    string Decompress(ImageDataHeader* imgHeader, vector<vector<uint8_t>>* colorTable, vector<uint8_t> codestream);

    /**
     * Initialize a code table based off the size of a given color table
     * 
     * @param colorTable 
     * @return std::unordered_map<int, std::string> Code Table
     */
    unordered_map<int, string> InitializeCodeTable(vector<vector<uint8_t>>* colorTable);
}

#endif // _LZW_H