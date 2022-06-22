#include "gif.h"

#include <iostream>
#include <tgmath.h>

GIF::GIF()
{

}

void GIF::ReadFileDataHeaders(const char* filepath)
{
    file = fopen(filepath, "rb");

    if (file == NULL) {
        fprintf(stderr, "Error opening file [%s]\n", filepath);
        exit(-1);
    }

    size_t filesize;

    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
    rewind(file);

    header = (Header*)malloc(sizeof(Header));
    fread(header, 1, sizeof(Header), file);

    lsd = (LogicalScreenDescriptor*)malloc(sizeof(LogicalScreenDescriptor));
    fread(lsd, 1, sizeof(LogicalScreenDescriptor), file);

    // Check to see if the GCT flag is set
    if (lsd->packed >> LSDFlags::GlobalColorTable) {
        printf("Global Color Table Present\n");

        gctd = (GlobalColorTableDescriptor*)malloc(sizeof(GlobalColorTableDescriptor));
        gctd->SizeInLSD = (lsd->packed >> LSDFlags::Size) & 0x07;
        gctd->NumberOfColors = pow(2, gctd->SizeInLSD + 1);
        gctd->ByteLegth = 3 * gctd->NumberOfColors;
        gct = malloc(gctd->ByteLegth);
        fread(gct, 1, gctd->ByteLegth, file);
    } else {
        printf("Global Color Table Not Present\n");
    }

    // gce = (GraphicsControlExtension*)malloc(sizeof(GraphicsControlExtension));
    // fread(gce, 1, sizeof(GraphicsControlExtension), file);

    
}

bool GIF::ValidHeader()
{
    for (int i = 0; i < 6; i++) {
        if (header->Signature[i] != GIF_MAGIC_0[i]
         && header->Signature[i] != GIF_MAGIC_1[i]) {
            return false;
        } 
    }

    return true;
}

void GIF::PrintHeaderInfo()
{   
    printf("\n------- GIF INFO -------\n");

    printf("[Header]\n");
    printf("\tSignature: %s\n", header->Signature);
    printf("\tVersion: %s\n", header->Version);

    printf("[Logical Screen Descriptor]\n");
    printf("\tWidth: %d\n", lsd->Width);
    printf("\tHeight: %d\n", lsd->Height);
    printf("\tGlobal Color Table Flag: %d\n", (lsd->packed >> LSDFlags::GlobalColorTable) & 0x1);
    printf("\tColor Resolution: %d\n", (lsd->packed >> LSDFlags::ColorResolution) & 0x07);
    printf("\tSort Flag: %d\n", (lsd->packed >> LSDFlags::Sort) & 0x01);
    printf("\tGlobal Color Table Size: %d\n", (lsd->packed >> LSDFlags::Size) & 0x07);
    printf("\tBackground Color Index: %d\n", lsd->BackgroundColorIndex);
    printf("\tPixel Aspect Ratio: %d\n", lsd->PixelAspectRatio);
    
    // printf("[Graphic Control Extension]\n");
    // printf("\tExtension Introducer: %X\n", gce->ExtensionIntroducer);
    // printf("\tGraphic Control Label: %d\n", gce->GraphicControlLabel);
    // printf("\tDisposal Method: %d\n", gce->DisposalMethod);
    // printf("\tUser Input Flag: %d\n", gce->UserInputFlag);
    // printf("\tTransparent Flag: %d\n", gce->TransparentFlag);
    // printf("\tDelay Time: %d\n", gce->DelayTime);
    // printf("\tTransparent Color Index: %d\n", gce->TransparentColorIndex);
    // printf("\tBlock Terminator: %d\n", gce->BlockTerminator);
    // printf("------------------------\n");
}