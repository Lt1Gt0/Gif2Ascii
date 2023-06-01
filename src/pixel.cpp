#include "pixel.hpp"

namespace GIF
{
    Pixel::Pixel(char sym, Color c)
    {
        symbol = sym;
        color = c;
    }

    void Pixel::PrintColor(FILE* fd)
    {
        fprintf(fd, "\x1b[48;2;%d;%d;%dm", color.red, color.blue, color.green);
        PrintChar(fd);
        fprintf(fd, "\x1b[0m");
    }

    void Pixel::PrintChar(FILE* fd)
    {
        fprintf(stdout, "\x1b[38;2;%d;%d;%dm", color.red, color.blue, color.green);
        fprintf(stdout, "%c", symbol);
        fprintf(fd, "\x1b[0m");
    }
}
