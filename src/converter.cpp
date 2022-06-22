#include <iostream>

#include "gif.h"

// unsigned int getBits(unsigned int x, int p, int n);

int main()
{
    const char* filepath = "imgs/transparent.gif";
    GIF gif = GIF();
    gif.ReadFileDataHeaders(filepath);

    if (!gif.ValidHeader()) {
        fprintf(stderr, "Invalid Header\n");   
        exit(-1);
    }

    printf("Vlaid file header\n");

    gif.PrintHeaderInfo();

    return 0;
}

// unsigned int getBits(unsigned int x, int p, int n)
// {
//     return (x >> (p + 1 - n) & ~(~0 << n));
// }