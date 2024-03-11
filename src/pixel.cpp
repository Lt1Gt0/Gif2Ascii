#include "pixel.hpp"

namespace GIF
{
    Pixel::Pixel(char sym, Color c, Position pos)
    {
        this->symbol = sym;
        this->color = c;
        this->position = pos;
    }

    void Pixel::PrintColor(FILE* fd)
    {
        fprintf(fd, "\x1b[48;2;%d;%d;%dm", color.red, color.green, color.blue);
        PrintChar(fd);
        fprintf(fd, "\x1b[0m");
    }

    void Pixel::PrintChar(FILE* fd)
    {
        fprintf(stdout, "\x1b[38;2;%d;%d;%dm", color.red, color.green, color.blue);
        fprintf(stdout, "%c", symbol);
        fprintf(fd, "\x1b[0m");
    }

    void Pixel::SetPos(Position pos)
    {
        this->position = pos;
    }
}
