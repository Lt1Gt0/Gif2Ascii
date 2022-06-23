#include <iostream>

#include "gif.h"

int main()
{
    const char* filepath = "imgs/sample_1.gif";
    
    FILE* fp = fopen(filepath, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file [%s]\n", filepath);
        exit(-1);
    } else {
        printf("Successfully opened [%s]\n", filepath);
    }

    GIF gif = GIF(fp);
    printf("Reading File Information Data...\n");
    gif.ReadFileDataHeaders();
    gif.PrintHeaderInfo();
    gif.DataLoop();

    return 0;
}