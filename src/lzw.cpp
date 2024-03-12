#include "lzw.hpp"
#include "utils/logger.hpp"
#include <stdio.h>
#include <string>
#include <math.h>
#include <fstream>
#include <stdio.h>

namespace LZW
{
    using namespace GIF::Data::Graphic;

    void DumpStream(std::vector<byte> stream, const char* filepath = "logs/lzw.log")
    {
        std::ofstream dump(filepath, std::ios::out | std::ios::app);

        dump.setf(std::ios::hex, std::ios::basefield);    
        for (byte b : stream) {
            dump << std::uppercase << (int)b << " ";
        }
        dump.unsetf(std::ios::hex);

        dump << std::endl;
        dump.close();
    }

    void DumpDecompressed(std::string_view decompressed, const char* filepath = "logs/lzw.log")
    {
        std::ofstream dump(filepath, std::ios::out | std::ios::app);
        dump << decompressed << std::endl;
        dump.close();
    }

    void DumpTable(std::unordered_map<int, std::string> table, const char* filepath = "logs/lzw.log")
    {
        std::ofstream dump(filepath, std::ios::out | std::ios::binary | std::ios::app);

        dump << "Table size: " << table.size() << std::endl;
        for (size_t i = 0; i < table.size(); i++) {
            dump << i << ": " << table[i].data() << std::endl;
        }

        dump.close();
    }

    std::string Decompress(const ImageData& imgData, const byte colorTableSize)
    {
        #ifdef DBG
        DumpStream(imgData.block.data);
        #endif

        if (imgData.block.data.size() <= 0)
            return "";

        logger.Log(DEBUG, "Decompressing stream...");

        std::vector<byte> data = imgData.block.data;
        std::unordered_map<int, std::string> table = InitializeCodeTable(colorTableSize);
        std::string charstream = ""; 

        int offset = 0;
        int i = 0;
        int codesize = imgData.lzwMinimum + 1;

        int newCode = (data[i] >> offset) & ((int)pow(2, codesize) - 1);
        int oldCode;

        offset += codesize;

        // Check for clearcode

        if (newCode == (int)std::pow(imgData.lzwMinimum, 2)) {
            logger.Log(DEBUG, "Encountered Clear Code...");
            table = InitializeCodeTable(colorTableSize);

            // #ifdef DBG
            // DumpTable(table);
            // #endif
        }

        newCode = (data[i] >> offset) & ((int)pow(2, codesize) - 1);
        offset += codesize;

        charstream += table[newCode];
        std::string s = table[newCode], c = "";
        c += s[0];

        oldCode = newCode;

        int count = table.size();
        while (i < (int)data.size() - 1) {
            if (offset + codesize > 8) {
                uint16_t tmp = (data[i + 1] << 8) | data[i];
                newCode = (tmp >> offset) & ((int)pow(2, codesize) - 1);
                offset -= 8;
                i++;
            } else {
                newCode = (data[i] >> offset) & ((int)pow(2, codesize) - 1);
            }

            offset += codesize;
            c = "";

            // If the code is not in the table
            if (table.find(newCode) == table.end()) {
                c += table[oldCode][0];
                s = table[oldCode] + c;
            } else {
                s = table[newCode];
                c = table[newCode][0];
            }

            if (table[newCode][0] == char(colorTableSize + 1)) {
                logger.Log(DEBUG, "Reinitializing Code table...");

                // #ifdef DBG
                // DumpTable(table);
                // #endif

                table = InitializeCodeTable(colorTableSize);
            }

            table[count] = table[oldCode] + c;
            count++;
            oldCode = newCode;

            charstream += s;

            if ((int)table.size() > (int)pow(2, codesize) - 1)
                codesize++;
        }

        return charstream;
    }

    std::unordered_map<int, std::string> InitializeCodeTable(const byte colorTableSize)
    {
        std::unordered_map<int, std::string> table;
        for (int i = 0; i < colorTableSize + SPECIAL_CODE_COUNT; i++) { 
            std::string ch = ""; 
            ch += char(i); 
            table[i] = ch; 
        }

        return table;
    }
}
