#include "colortounicode.h"

char* BrightnessToUnicode(double brightness)
{
    char* projectedChar;

    // Nah brightness dont matter
    if ((int)brightness >= 25 && (int)brightness <= 50) {
        projectedChar = (char*)"-"; // PlaceHolder
        // projectedChar = (char*)u8"\u0D9E";
    } else if ((int)brightness > 50) {
        // projectedChar = (char*)u8"\u0DB0";
        projectedChar = (char*)"|"; // PlaceHolder
    } else {
        projectedChar = (char*)" ";
    }

    return projectedChar;
}