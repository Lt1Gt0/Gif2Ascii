#include <iostream>

#include "gif.h"

// unsigned int getBits(unsigned int x, int p, int n);

int main()
{
    const char* filepath = "imgs/transparent.gif";
    
    FILE* fp = fopen(filepath, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file [%s]\n", filepath);
        exit(-1);
    } else {
        printf("Successfully opened [%s]\n", filepath);
    }

    GIF gif = GIF(fp);

    printf("\nValidating GIF Header...\n");
    if (gif.LoadHeader() == -1) {
        fprintf(stderr, "Invalid GIF Header\n");   
        exit(-1);
    } else {
        printf("Valid GIF header\n\n");
    }

    printf("Reading File Information Data...\n");
    gif.ReadFileDataHeaders();

    gif.PrintHeaderInfo();

    // printf("0x%lX\n", ftell(gif.file));
    gif.DataLoop();

    return 0;
}

// unsigned int getBits(unsigned int x, int p, int n)
// {
//     return (x >> (p + 1 - n) & ~(~0 << n));
// }