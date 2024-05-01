#include "lzw.hpp"
#include "utils/logger.hpp"
#include <stdio.h>
#include <string>
#include <math.h>

namespace LZW
{
    string Decompress(const ImageDataHeader& imgHeader, const uint8_t colorTableSize, vector<uint8_t> codestream)
    {
        if (codestream.size() <= 0)
            return "";

        logger.Log(DEBUG, "Decompressing stream...");

        unordered_map<int, string> table;
        string charstream = ""; 
        table = InitializeCodeTable(colorTableSize);

        int offset, newCode, oldCode, codesize, i;
        offset = 0, i = 0;
        codesize = imgHeader.LZWMinimum + 1;

        newCode = (codestream[i] >> offset) & ((int)pow(2, codesize) - 1);
        offset += codesize;

        // Check for clearcode
        if (newCode == 4) {
            logger.Log(DEBUG, "Encountered Clear Code...");
            table = InitializeCodeTable(colorTableSize);
        }

        newCode = (codestream[i] >> offset) & ((int)pow(2, codesize) - 1);
        offset += codesize;

        charstream += table[newCode];
        string s = table[newCode], c = "";
        c += s[0];

        oldCode = newCode;

        int count = table.size();
        while (i < (int)codestream.size() - 1) {
            if (offset + codesize > 8) {
                uint16_t tmp = (codestream[i + 1] << 8) | codestream[i];
                newCode = (tmp >> offset) & ((int)pow(2, codesize) - 1);
                offset -= 8;
                i++;
            } else {
                newCode = (codestream[i] >> offset) & ((int)pow(2, codesize) - 1);
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
                table = InitializeCodeTable(colorTableSize);
            }

            table[count] = table[oldCode] + c;
            count++;
            oldCode = newCode;

            charstream += s;

            if ((int)table.size() > (int)pow(2, codesize) - 1)
                codesize++;
        }

        // printf("%s\n", charstream);
        return charstream;
    }

    unordered_map<int, string> InitializeCodeTable(const uint8_t colorTableSize)
    {
        unordered_map<int, string> table;

        for (int i = 0; i < colorTableSize + SPECIAL_CODE_COUNT; i++) { 
            string ch = ""; 
            ch += char(i); 
            table[i] = ch; 
        }

        return table;
    }
}
