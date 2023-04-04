#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "ParseBmp.h"

const size_t kPixelSizeOffset   = 0x1C;
const size_t kStartPixelsOffset = 0xA;
const size_t kBcWidthOffset     = 0x12;
const size_t kBcHeightOffset    = 0x16;

static long GetTextSize(const char *file_name);

void GetImageFromBMP(Image_t* image, const char* file_path)
{
    int fd = open(file_path, O_RDONLY);

    char* pixel_array = (char*)mmap(NULL, (size_t)GetTextSize(file_path), PROT_READ, MAP_SHARED, fd, 0);

    size_t pixels_start_offset = 0;
    memcpy(&pixels_start_offset, &pixel_array[kStartPixelsOffset], sizeof(int));

    memcpy(&image->width,  &pixel_array[kBcWidthOffset],  2);     //
    memcpy(&image->height, &pixel_array[kBcHeightOffset], 2);    //get size of image
    
    image->pixels = (Pixel_t*)calloc(image->width*image->height, sizeof(Pixel_t));

    size_t pixel_size = 0;
    memcpy(&pixel_size, &pixel_array[kPixelSizeOffset], 2);
    pixel_size /= 8;        //convert from bit to byte

    #ifdef DEBUG
        printf("height      = %d\n", image->height);
        printf("width       = %d\n", image->width);
        printf("pixel_size  = %d\n", pixel_size);
    #endif

    if (pixel_size == 3)
    {
        size_t image_arr_size = image->width * image->height;

        for (size_t i = 0; i < image_arr_size; i++)
        {
            image->pixels[i].r = pixel_array[pixels_start_offset + i*3 + 2];
            image->pixels[i].g = pixel_array[pixels_start_offset + i*3 + 1];
            image->pixels[i].b = pixel_array[pixels_start_offset + i*3];
            image->pixels[i].a = (char)0xFF;
        }
    }
    else if (pixel_size == 4)
    {   
        memcpy(image->pixels, &pixel_array[pixels_start_offset], image->width * image->height * 4);
    }
    else
    {
        fprintf(stderr, "Wrong file format\n");
        return;
    }
}

static long int GetTextSize(const char *file_name)
{
    struct stat buff = {};
    stat(file_name, &buff);
    return buff.st_size;
}