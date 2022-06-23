#include "image.h"

#include <stdio.h>
#include <unordered_map>
#include "lzw.h"
#include "colortounicode.h"

Image::Image(FILE* fp, std::vector<std::vector<uint8_t>>* colortable)
{
    this->file = fp;
    this->colorTable = colortable;
    // this.GifWidth = gifWidth;
    // this->GifHeight = gifHeight;
}

void Image::ReadDataSubBlocks(FILE* file)
{
    data.resize(header->FollowSize);

    // Read the first sub block
    for (int i = 0; i < (int)data.size(); i++) {
        fread(&data[i], 1, 1, file);
    }
    
    uint8_t* nextByte = (uint8_t*)malloc(sizeof(uint8_t));
    int dataSize;
    fread(nextByte, 1, 1, file);

    while (true) {
        // Check for the end of sub block
        if (*nextByte == 0x00)
            break;
        
        // Resize the vector to account for the new block to be added
        dataSize = (int)data.size();
        data.resize(dataSize + (int)*nextByte);

        // Read the next (*nextByte) bytes into the vector
        for (int i = 0; i < *nextByte; i++) {
            fread(&data[i + dataSize], 1, 1, file);
        }
        
        fread(nextByte, 1, 1, file);
    }
    
    for (uint8_t d : data) {
        printf("%X ", d);
    }
    printf("\n");

    free(nextByte);
}

void Image::LoadExtension(ExtensionHeader* header)
{
    // If the header has a valid label for an extension, start each one
    // by loading the struct into memory and set its header to the one that was passed in
    switch (header->Label) {
    case ExtensionTypes::PlainText:
    {
        printf("Loding Plain Text Extension\n");

        PlainTextExtension* pt = (PlainTextExtension*)malloc(sizeof(PlainTextExtension));
        pt->header = *header;

        // Load the block size into the struct and load the data of that size into the data buffer
        fread(&pt->BlockSize, 1, 1, file);
        printf("Block Size: %d\n", pt->BlockSize);
        uint8_t* dataBuf = (uint8_t*)malloc(sizeof(uint8_t) * pt->BlockSize);
        fread(dataBuf, 1, pt->BlockSize, file);
        pt->Data = dataBuf;

        extensions->PlainText = pt;

    } break;
    case ExtensionTypes::GraphicsControl:
    {
        printf("Loading Graphics Control Extension\n");

        // I feel stupid now because I kept reading over the header thinking it was not part of the extension read
        fseek(file, -2, SEEK_CUR);
        GraphicsControlExtension* gce = (GraphicsControlExtension*)malloc(sizeof(GraphicsControlExtension));

        // Read the bytes following the header since the header is already loaded
        fread(gce, 1, sizeof(GraphicsControlExtension), file);
        extensions->GraphicsControl = gce;
        
        printf("GCE Packed: %d\n", extensions->GraphicsControl->Packed);
        // printf("Loaded Graphics Control Extension\n");
    } break;
    case ExtensionTypes::Comment:
    {
        printf("Loding Comment Extension\n");

        CommentExtension* ce = (CommentExtension*)malloc(sizeof(CommentExtension));
        ce->Header = *header;

        // Read a store bytes until 0x00 is hit
        uint8_t* nextByte = (uint8_t*)malloc(sizeof(uint8_t));
        for (int i = 0; *nextByte != 0x00; i++) {
            fread(nextByte, 1, 1, file);
            // ce->Data[i] = *nextByte;
        }
        
        extensions->Comment = ce;
    } break;
    case ExtensionTypes::Application:
    {
        // So it turns out the application extension is very important for what I need because
        // its for looping the animation 
        printf("Loding Application Extension\n");

        ApplicationExtension* ae = (ApplicationExtension*)malloc(sizeof(ApplicationExtension));
        ae->Header = *header;

        // Get the block length size to follow
        fread(&ae->BlockLength, 1, 1, file);
        printf("Application Block Length: %d\n", ae->BlockLength);

        // Load the Application Identifier
        ae->Identifier = (uint8_t*)malloc(sizeof(uint8_t) * ae->BlockLength);
        fread(&ae->Identifier, 1, ae->BlockLength, file);

        // Load the Application Authentication
        uint8_t authLength;
        fread(&authLength, 1, 1, file);
        ae->AuthenticationCode = (uint8_t*)malloc(sizeof(uint8_t) * authLength);
        fread(ae->AuthenticationCode, 1, authLength, file);

        // Check to see if the end of the Application Extension has been reached
        uint8_t* next = (uint8_t*)malloc(sizeof(uint8_t));
        fread(next, 1, 1, file);
        if (*next == 0x00) {
            printf("Application block end\n");
        }

        extensions->Application = ae;
    } break;
    default:
    {
        fprintf(stderr, "Recived Invalid extension type [%X]\n", header->Label);
    } break;
    }
}

std::string Image::LoadImageData()
{
    // Load the Image Descriptor into memory
    descriptor = (ImageDescriptor*)malloc(sizeof(ImageDescriptor));
    fread(descriptor, 1, sizeof(ImageDescriptor), file);

    // Load the Local Color Table if it is set 
    // The current GIF I am trying to target does not have a need for a LCT so I am just
    // going to skip loading one for now (sucks to suck)
    if ((descriptor->Packed >> ImgDescMask::LocalColorTable) & 0x1)
        printf("Loading Local color Table\n");
    else 
        printf("No Local Color Table Flag Set\n");

    // Load the image header into memory
    header = (ImageDataHeader*)malloc(sizeof(ImageDataHeader));
    fread(header, 1, 2, file); // Only read 2 bytes of file steam for LZW min and Follow Size 

    // PrintData();
    ReadDataSubBlocks(file);

    // Get the raster data from the image frame by decompressing the data block from the gif
    std::string rasterData = LZW::Decompress(header, colorTable, data);
    return rasterData;
}

void Image::CheckExtensions()
{
    printf("\nChecking for extensions...\n");
    fpos_t prevPos;

    // Load a dummy header into memory
    ExtensionHeader* extensionCheck = (ExtensionHeader*)malloc(sizeof(ExtensionHeader));

    while (true) {
        fgetpos(file, &prevPos);
        fread(extensionCheck, 1, sizeof(ExtensionHeader), file);

        // If the dummy header contains an introducer for a extension, load the extension type
        if (extensionCheck->Introducer == EXTENSION_INTRODUCER) {
            printf("Found Extension Introducer\n");
            LoadExtension(extensionCheck);
        } else {
            printf("Ending Extension Check\n");
            fsetpos(file, &prevPos);
            free(extensionCheck);
            return;
        }
    }
}

void Image::ParseGCE()
{
    if (extensions->GraphicsControl == NULL) {
        printf("Graphics Control extensions not found...\n");
        return;
    }

    uint8_t disposalMethod = (extensions->GraphicsControl->Packed >> Disposal) & 0x07;
    uint8_t userInput = (extensions->GraphicsControl->Packed >> UserInput) & 0x01;
    uint8_t transparentFlag = (extensions->GraphicsControl->Packed >> TransparentColor) & 0x07;

    // According to GIF89a standard, Disposal method has 7 different possibilities
    // 0 -> If the image was not animated the bits would be 0 to specify no disposal method
    // 1 -> Decoder hould leave the image in place and draw the next image on top of it
    // 2 -> Canvas should be restored to the background color (indicated by the LSD)
    // 3 -> Decoder should retore the canvas to its previous state before the current image was drawn
    // 4 - 7 -> Undefined (I dont know why its a 3 bit field then, I guess maybe for later use)
}

void Image::UpdateFrame(std::string* rasterData, std::vector<char>* pixelMap)
{
    // Initialize a code table so I know what color/code to output
    // std::unordered_map<int, std::string> codeTable = LZW::InitializeCodeTable(colorTable);

    // Because each gif can have a different disposal method for different frames (according to GIF89a)
    // it is best to handle each disposal method instread of printing the decompressed codestream directly
    int disposalMethod = ((extensions->GraphicsControl->Packed >> Disposal) & 0x07);
    printf("Disposal Method: %d\n", disposalMethod);
    switch (disposalMethod) {
    case 0:
        break;
    case 1:
        DrawOverImage(rasterData, pixelMap);
        break;
    case 2:
        RestoreCanvasToBG(rasterData, pixelMap);
        break;
    case 3:
        RestoreToPrevState(rasterData, pixelMap);
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        break;
    default:
        fprintf(stderr, "Undefined Disposal Method: %d\n", disposalMethod);
        exit(-1);
        break;
    }
    
    
    // Initialize variables for Image Display
    // int col = 0, sum = 0;
    // double averageBrightness;

    // for (char c : *rasterData) {
    //     if (col >= descriptor->Width) {
    //         col = 0;
    //         fprintf(stderr, "\n");
    //     }

    //     // if (c == codeTable[(int)codeTable->size() - 1][0].c_str()) {
    //     if (c == codeTable.at((int)codeTable.size() - 1)[0]) {
    //         printf("%c - End of Information\n", c);
    //         break;
    //     }

    //     // If for some reason a character below 'A' is encountered then leave the loop
    //     if ((int)c - 'A' < 0) 
    //         break;

    //     std::vector<uint8_t> color = colorTable->at((int)c - 'A');
        
    //     // Get the sum of each color brightness at the current code
    //     for(uint8_t c : color) {
    //         sum += (int)c;
    //     }

    //     averageBrightness = sum / color.size();
    //     // printf("Average Brightness: %f\n", averageBrightness);
    //     fprintf(stderr, "%s", BrightnessToUnicode(averageBrightness));

    //     col++;
    //     sum = 0;
    // }
    // printf("\n");
}

void Image::PrintDescriptor()
{
    printf("------- Image Descriptor -------\n");
    printf("Seperator: %X\n", descriptor->Seperator);
    printf("Image Left: %d\n", descriptor->Left);
    printf("Image Top: %d\n", descriptor->Top);
    printf("Image Width: %d\n", descriptor->Width);
    printf("Image Height: %d\n", descriptor->Height);
    printf("Local Color Table Flag: %d\n", (descriptor->Packed >> ImgDescMask::LocalColorTable) & 0x1);
    printf("Interlace Flag: %d\n", (descriptor->Packed >> ImgDescMask::Interlace) & 0x1);
    printf("Sort Flag: %d\n", (descriptor->Packed >> ImgDescMask::IMGSort) & 0x1);
    printf("Size of Local Color Table: %d\n", (descriptor->Packed >> ImgDescMask::IMGSize) & 0x7);
    printf("--------------------------------\n");
}

void Image::PrintData()
{
    printf("\n------- Image Data -------\n");
    printf("LZW Minimum: 0x%X\n", header->LZWMinimum);
    printf("Initial Follow Size: 0x%X\n", header->FollowSize);
    printf("--------------------------\n");
}

void Image::PrintSubBlockData(std::vector<uint8_t> block)
{
    printf("\n------- Block Data -------\n");
    printf("Size: %ld\n", block.size());
    for (int i = 0; i < (int)block.size(); i++) {
        printf("%X ", block.at(i));
    }
    printf("\n--------------------------\n");
}

void Image::DrawOverImage(std::string* rasterData, std::vector<char>* pixelMap)
{
    printf("Drawing Over Image\n");
    // int offset = (descriptor->Top * this->GifWidth) + descriptor->Left;

    // printf("Image offset %d\n", offset);
}

void Image::RestoreCanvasToBG(std::string* rasterData, std::vector<char>* pixelMap)
{
    printf("Restore Canvas to BackGround\n");
}

void Image::RestoreToPrevState(std::string* rasterData, std::vector<char>* pixelMap)
{
    printf("Restore Canvas to Previous State\n");
}