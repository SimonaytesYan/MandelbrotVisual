#ifndef SYM_PARSE_BMP
#define SYM_PARSE_BMP

#include <stdlib.h>

struct Pixel_t
{
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    unsigned char a = 0;
};

struct BmpFileInfo
{
    size_t pixel_size   = 0;
    size_t pixel_offset = 0;
    size_t w            = 0;
    size_t h            = 0;
};

struct Image_t
{
    Pixel_t*    pixels = nullptr;
    BmpFileInfo info   = {};
};

void GetImageFromBMP(Image_t* image, const char* file_path);

#endif