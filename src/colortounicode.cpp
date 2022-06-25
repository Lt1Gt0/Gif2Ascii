#include "colortounicode.h"

char* BrightnessToUnicode(double brightness)
{
    char* projectedChar;

    // Nah brightness dont matter
    if ((int)brightness >= 50 && (int)brightness < 70) {
        projectedChar = (char*)"-"; // PlaceHolder
        // projectedChar = (char*)u8"\u0D9E";
    } else if ((int)brightness > 75) {
        // projectedChar = (char*)u8"\u0DB0";
        projectedChar = (char*)"|"; // PlaceHolder
    } else {
        projectedChar = (char*)" ";
    }

    return projectedChar;
}

char* ColorToUnicode(std::vector<uint8_t>* color)
{
    char* projectedChar;

    if (color->at(0) && !color->at(1) && !color->at(2)) { // Check Red
        // projectedChar = (char*)u8"\u0D9E";
        projectedChar = (char*)"-"; // PlaceHolder
        // projectedChar = (char*)" ";
    } else if (color->at(1) && !color->at(0) && !color->at(2)) { // Check Green
        projectedChar = (char*)"|"; // PlaceHolder
        // projectedChar = (char*)u8"\u0DAC";
    } else if (color->at(0) && color->at(1) && !color->at(2)) { // Check Yellow
        projectedChar = (char*)"~"; // PlaceHolder
        // projectedChar = (char*)u8"\u0DB0";
    } else if (!color->at(1) && !color->at(1) && !color->at(2)) { // Check Black
        projectedChar = (char*)"="; // PlaceHolder
        // projectedChar = (char*)u8"\u0DB0";
        // projectedChar = (char*)u8"\u0D9E";
    } else {
        projectedChar = (char*)" ";
    }

    return projectedChar;
}