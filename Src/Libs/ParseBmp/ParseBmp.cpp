#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "ParseBmp.h"

const size_t kStartPixelsOffset = 0xA;
const size_t kBcWidthOffset     = 0x12;
const size_t kBcHeightOffset    = 0x16;
const size_t kPixelSizeOffset   = 0x1C;
const size_t kBmpHeaderSize     = 0x36;

static void GetBmpFileInfo(BmpFileInfo* result, const char* file_path);
static long GetTextSize(const char *file_name);

void GetImageFromBMP(Image_t* image, const char* file_path)
{
    GetBmpFileInfo(&image->info, file_path);

    int fd = open(file_path, O_RDONLY);
    char* pixel_array = (char*)mmap(NULL, image->info.w*image->info.h * image->info.pixel_size,
                                    PROT_READ, MAP_SHARED, fd, image->info.pixel_offset);
    
    image->pixels = (Pixel_t*)calloc(image->info.w*image->info.h, sizeof(Pixel_t));

    #ifdef DEBUG
        printf("h      = %d\n", image->info.h);
        printf("w       = %d\n", image->info.w);
        printf("pixel_size  = %d\n", pixel_size);
    #endif

    if (image->info.pixel_size == 3)
    {
        size_t image_arr_size = image->info.w * image->info.h;

        for (size_t i = 0; i < image_arr_size; i+=3)
        {
            image->pixels[i].r = pixel_array[i + 2];
            image->pixels[i].g = pixel_array[i + 1];
            image->pixels[i].b = pixel_array[i];
            image->pixels[i].a = (char)0xFF;
        }
    }
    else if (image->info.pixel_size == 4)
    {   
        memcpy(image->pixels, pixel_array, image->info.w * image->info.h * image->info.pixel_size);
    }
    else
    {
        fprintf(stderr, "Wrong file format\n");
        return;
    }
}

static void GetBmpFileInfo(BmpFileInfo* result, const char* file_path)
{
    int   fd        = open(file_path, O_RDONLY);
    char* file_info = (char*)mmap(NULL, kBmpHeaderSize, PROT_READ, MAP_SHARED, fd, 0);

    memcpy(&result->w,  &file_info[kBcWidthOffset],  2);                    //
    memcpy(&result->h, &file_info[kBcHeightOffset], 2);                    //get size of image

    memcpy(&result->pixel_offset, &file_info[kStartPixelsOffset], sizeof(int)); //get start of the pixel array

    memcpy(&result->pixel_size,   &file_info[kPixelSizeOffset],   2);           //get numbet bit in one pixel
    result->pixel_size /= 8;                                                    //convert from bit to byte
}

static long int GetTextSize(const char *file_name)
{
    struct stat buff = {};
    stat(file_name, &buff);
    return buff.st_size;
}