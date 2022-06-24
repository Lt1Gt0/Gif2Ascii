#include "lzw.h"
#include <stdio.h>
#include <string>
#include <math.h>

std::string LZW::Decompress(ImageDataHeader* imgHeader, std::vector<std::vector<uint8_t>>* colorTable, std::vector<uint8_t> codestream)
{
    fprintf(stdout, "Decompressing stream...\n");

    std::unordered_map<int, std::string> table;
    std::string charstream = ""; 
    table = InitializeCodeTable(colorTable);

    int offset, newCode, oldCode, codesize, i;
    offset = 0, i = 0;
    codesize = imgHeader->LZWMinimum + 1;

    newCode = (codestream[i] >> offset) & ((int)pow(2, codesize) - 1);
    offset += codesize;

    // Check for clearcode
    if (newCode == 4) {
        // fprintf(stdout, "Encountered Clear Code...\n");
        table = InitializeCodeTable(colorTable);
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
        
        if (table[newCode][0] == char(colorTable->size() + 1)) {
            // fprintf(stdout, "\nReinitializing Code table...\n");
            table = InitializeCodeTable(colorTable);
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

std::unordered_map<int, std::string> LZW::InitializeCodeTable(std::vector<std::vector<uint8_t>>* colorTable)
{
    std::unordered_map<int, std::string> table;
    
    for (int i = 0; i < (int)colorTable->size() + 2; i++) { 
        std::string ch = ""; 
        ch += char(i); 
        table[i] = ch; 
    }

    return table;
}