#include "lzw.h"
#include "logger.h"
#include <math.h>

namespace LZW
{
    std::string Decompress(const GIF::ImageDataHeader& imgHeader, const byte colorTableSize, std::vector<byte> codestream)
    {
        if (codestream.size() <= 0)
            return "";

        LOG_DEBUG << "Decompressing stream" << std::endl;

        std::unordered_map<int, std::string> table;
        std::string charstream = ""; 
        table = InitializeCodeTable(colorTableSize);

        int offset, newCode, oldCode, codesize, i;
        offset = 0, i = 0;
        codesize = imgHeader.LZWMinimum + 1;

        newCode = (codestream[i] >> offset) & ((int)pow(2, codesize) - 1);
        offset += codesize;

        // Check for clearcode
        if (newCode == 4) {
            LOG_DEBUG << "Encountered Clear Code" << std::endl;
            table = InitializeCodeTable(colorTableSize);
        }

        newCode = (codestream[i] >> offset) & ((int)pow(2, codesize) - 1);
        offset += codesize;

        charstream += table[newCode];
        std::string s = table[newCode], c = "";
        c += s[0];

        oldCode = newCode;

        int count = table.size();
        while (i < (int)codestream.size() - 1) {
            if (offset + codesize > 8) {
                word tmp = (codestream[i + 1] << 8) | codestream[i];
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
                LOG_DEBUG << "Reinitializing Code table" << std::endl;
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
