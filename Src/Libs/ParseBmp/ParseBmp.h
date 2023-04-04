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

struct Image_t
{
    Pixel_t* pixels = nullptr;
    size_t   width  = 0;
    size_t   height = 0;
};

void GetImageFromBMP(Image_t* image, const char* file_path);

#endif