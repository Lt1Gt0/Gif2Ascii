#pragma once
#ifndef _LZW_H
#define _LZW_H

#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <string>

#include "imagedata.h"

namespace LZW 
{
    void Decompress(ImageData::Image* img, std::vector<std::vector<uint8_t>> colorTable, std::vector<uint8_t> codestream);
    std::unordered_map<int, std::string> InitializeCodeTable(ImageData::Image* img, std::vector<std::vector<uint8_t>> colorTable);
}

#endif // _LZW_H