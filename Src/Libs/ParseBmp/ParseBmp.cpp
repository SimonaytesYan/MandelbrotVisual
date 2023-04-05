#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ParseBmp.h"

#define DEBUG

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

    #ifdef DEBUG
        printf("h            = %d\n",   image->info.h);
        printf("w            = %d\n",   image->info.w);
        printf("pixel_size   = %d\n",   image->info.pixel_size);
        printf("pixel_offs   = %d\n\n", image->info.pixel_offset);
    #endif

    int   fd          = open(file_path, O_RDONLY);
    char* pixel_array = (char*)mmap(NULL, image->info.w * image->info.h * image->info.pixel_size + image->info.pixel_offset, 
                                    PROT_READ, MAP_PRIVATE, fd, 0);

    if (pixel_array == MAP_FAILED)
    {
        fprintf(stderr, "error during open file\n");
        switch (errno)
        {
            case EINVAL:
                fprintf(stderr, "EINVAL\n");
                break;
            case EACCES:
                fprintf(stderr, "EACCES\n");
                break;
            case EAGAIN:
                fprintf(stderr, "EAGAIN\n");
                break;
            case EBADF:
                fprintf(stderr, "EBADF\n");
                break;
            case EEXIST:
                fprintf(stderr, "EEXIST\n");
                break;
            case ENFILE:
                fprintf(stderr, "ENFILE\n");
                break;
            case ENOMEM:
                fprintf(stderr, "ENOMEM\n");
                break;
            case EOVERFLOW:
                fprintf(stderr, "EOVERFLOW\n");
                break;
            case EPERM:
                fprintf(stderr, "EPERM\n");
                break;
            case ETXTBSY:
                fprintf(stderr, "ETXTBSY\n");
                break;            
            default:
                break;
        }
        return;
    }
    
    image->pixels = (Pixel_t*)calloc(image->info.w * image->info.h, sizeof(Pixel_t));

    if (image->info.pixel_size == 3)
    {
        size_t image_arr_size = image->info.w * image->info.h;
        for (size_t i = 0; i < image_arr_size; i++)
        {
            image->pixels[i].r = pixel_array[i*3 + 2 + image->info.pixel_offset];
            image->pixels[i].g = pixel_array[i*3 + 1 + image->info.pixel_offset];
            image->pixels[i].b = pixel_array[i*3     + image->info.pixel_offset];
            image->pixels[i].a = (char)0xFF;
        }
    }
    else if (image->info.pixel_size == 4)
    {   
        memcpy(image->pixels, &pixel_array[image->info.pixel_offset], image->info.w * image->info.h * image->info.pixel_size);
    }
    else
    {
        fprintf(stderr, "Wrong file format\n");
        return;
    }

    munmap(pixel_array, image->info.w * image->info.h * image->info.pixel_size + image->info.pixel_offset);                                          
    close(fd); 
}

static void GetBmpFileInfo(BmpFileInfo* result, const char* file_path)
{
    int   fd        = open(file_path, O_RDONLY);
    char* file_info = (char*)mmap(NULL, kBmpHeaderSize, PROT_READ, MAP_PRIVATE, fd, 0);

    memcpy(&result->w,  &file_info[kBcWidthOffset], 2);                         //
    memcpy(&result->h, &file_info[kBcHeightOffset], 2);                         //get size of image

    memcpy(&result->pixel_offset, &file_info[kStartPixelsOffset], sizeof(int)); //get start of the pixel array

    memcpy(&result->pixel_size,   &file_info[kPixelSizeOffset],   2);           //get numbet bit in one pixel
    result->pixel_size /= 8;                                                    //convert from bit to byte

    munmap(file_info, kBmpHeaderSize);                                          
    close(fd);  
}

static long int GetTextSize(const char *file_name)
{
    struct stat buff = {};
    stat(file_name, &buff);
    return buff.st_size;
}