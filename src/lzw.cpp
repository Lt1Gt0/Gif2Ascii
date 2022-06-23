#include "lzw.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <math.h>

void LZW::Decompress(ImageData::Image* img, std::vector<std::vector<uint8_t>> colorTable, std::vector<uint8_t> codestream)
{
    printf("Decompressing stream...\n");

     // Assume these values are what were given in the image header
    // int lzwMinimum = 2;
    // int colorCount = 4;

    std::cout << "Decoding...\n"; 
    std::unordered_map<int, std::string> table; 
    int i = 0;
    for (; i < (int)colorTable.size(); i++) { 
        std::string ch = ""; 
        ch += char(i) + 'A'; 
        table[i] = ch; 
    }

    table[i] = char(i) + 'A';
    table[++i] = char(i + 1) + 'A';

    int offset, newCode, oldCode, codesize;
    offset = 0;
    codesize = img->header->LZWMinimum + 1;
    i = 0;

    newCode = (codestream[i] >> offset) & ((int)pow(2, codesize) - 1);
    offset += codesize;

    if (newCode == 4) {
        std::cout << "Encountered Clear Code...\n";
        table.clear();
        for (; i < (int)colorTable.size(); i++) {
            std::string ch = "";
            ch += char(i) + 'A';
            table[i] = ch;
        }

        table[i] = char(i) + 'A';
        table[++i] = char(i + 1) + 'A';

        i = 0;
    }

    newCode = (codestream[i] >> offset) & ((int)pow(2, codesize) - 1);
    offset += codesize;

    std::cout << table[newCode]; // Sould print 'B' if sinve it's the next code after 'E'
    std::string s = table[newCode];
    std::string c = "";
    c += s[0];

    oldCode = newCode;

    int count = table.size();

    while (i < (int)codestream.size() - 1) {
        if (offset + codesize > 8) {
            // offset += codesize;
            uint16_t tmp = (codestream[i + 1] << 8) | codestream[i];
            newCode = (tmp >> offset) & ((int)pow(2, codesize) - 1);
            offset -= 8;
            offset += codesize;
            i++;
        } else {
            newCode = (codestream[i] >> offset) & ((int)pow(2, codesize) - 1);
            offset += codesize;
        }

        if (table.find(newCode) == table.end()) {
            c = "";
            c += table[oldCode][0];
            s = table[oldCode] + c;
            table[count] = table[oldCode] + c;
            count++;
            oldCode = newCode;
        } else {
            s = table[newCode];
            c = "";
            c = table[newCode][0];
            table[count] = table[oldCode] + c;
            count++;
            oldCode = newCode;
        }

        std::cout << s;

        if ((int)table.size() > (int)pow(2, codesize) - 1) {
            codesize++;
        }
    }
    std::cout << "\n";
}

std::unordered_map<int, std::string> LZW::InitializeCodeTable(ImageData::Image* img, std::vector<std::vector<uint8_t>> colorTable)
{
    printf("Initializing code table\n");
    std::unordered_map<int, std::string> table;

    // Add 2 to the LZW minimum to account for CC and EOI
    for(int i = 0; i < (int)pow(img->header->LZWMinimum,2) + 2; i++) {
        table[i] = std::string(1, (char)('A' + i));
    }

    // for (int i = 0; i <= 255; i++) {
    //     std::string ch = "";
    //     ch += char(i);
    //     table[i] = ch;
    // }

    // Printing table for debug purposes
    for (auto x : table) {
        printf("%d: ", x.first);

        for (uint8_t c : x.second) {
            printf("%c ", c);
        }

        printf("\n");
    }
    printf("-------------------\n");
    return table;
}