#include "lzw.hpp"
#include "utils/logger.hpp"
#include <stdio.h>
#include <string>
#include <math.h>
#include <fstream>
#include <stdio.h>

namespace LZW
{
    void DumpStream(std::vector<byte> stream, const char* filepath = "logs/lzw.log")
    {
        std::ofstream dump(filepath, std::ios::out | std::ios::binary | std::ios::app);

        char* buf = new char[sizeof(byte) * 3];
        for (byte b : stream) {
            sprintf(buf, "%02X", b);
            dump << buf << " ";
        }

        dump << std::endl;
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

    std::string Decompress(const GIF::ImageDataHeader& imgHeader, const byte colorTableSize, std::vector<byte> codestream)
    {
        #ifdef DBG
        DumpStream(codestream);
        #endif

        if (codestream.size() <= 0)
            return "";

        logger.Log(DEBUG, "Decompressing stream...");

        std::unordered_map<int, std::string> table;
        std::string charstream = ""; 
        table = InitializeCodeTable(colorTableSize);

        int offset = 0;
        int i = 0;
        int codesize = imgHeader.LzwMinimum + 1;

        int newCode = (codestream[i] >> offset) & ((int)pow(2, codesize) - 1);
        int oldCode;

        offset += codesize;

        // Check for clearcode
        if (newCode == 4) {
            logger.Log(DEBUG, "Encountered Clear Code...");
            table = InitializeCodeTable(colorTableSize);

            #ifdef DBG
            DumpTable(table);
            #endif
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

            if (table[newCode][0] == char(colorTableSize + 1)) {
                logger.Log(DEBUG, "\nReinitializing Code table...");

                #ifdef DBG
                DumpTable(table);
                #endif

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
