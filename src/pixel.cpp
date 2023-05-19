#include "pixel.hpp"

namespace GIF
{
    Pixel::Pixel(char sym, Color c)
    {
        mSymbol = sym;
        mColor = c;
    }

    void Pixel::PrintColor(FILE* fd)
    {
        fprintf(fd, "\x1b[48;2;%d;%d;%dm", mColor.Red, mColor.Blue, mColor.Green);
        PrintChar(fd);
        fprintf(fd, "\x1b[0m");
    }

    void Pixel::PrintChar(FILE* fd)
    {
        fprintf(stdout, "\x1b[38;2;%d;%d;%dm", mColor.Red, mColor.Blue, mColor.Green);
        fprintf(stdout, "%c", mSymbol);
        fprintf(fd, "\x1b[0m");
    }
}
