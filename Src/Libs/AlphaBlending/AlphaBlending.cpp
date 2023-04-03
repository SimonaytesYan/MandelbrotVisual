#include <fcntl.h>
#include <SFML/Graphics.hpp>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "AlphaBlending.h"
#include "../Stopwatch.h"

#define DRAW

const size_t kPixelSizeOffset   = 0x1C;
const size_t kStartPixelsOffset = 0xA;
const size_t kBcWidthOffset     = 0x12;
const size_t kBcHeightOffset    = 0x16;

const size_t kMaxFpsStrLen = 20;

const char   kWindowHeader[]     = "Alpha blending";
const size_t kTimeCalcAlphaBlend = 1;

static void UpdateFpsViewer(sf::Text *fps_counter, float fps);
static int GetTextSize(const char *file_name);

static void GetImageFromBMP(Image_t* image, const char* file_path);

void MakeAlphaBlending(char* background_path, char* foreground_path)
{
    Image_t background = {};
    Image_t foreground = {};

    GetImageFromBMP(&background, background_path);
    GetImageFromBMP(&foreground, foreground_path);

    Image_t result = {};
    result.pixels = (Pixel_t*)calloc(sizeof(Pixel_t), background.height * background.width);
    result.height = background.height;
    result.width  = background.width;

    memcpy(result.pixels, background.pixels, background.height * background.width * sizeof(Pixel_t));

    sf::RenderWindow window(sf::VideoMode(background.width, background.height), kWindowHeader, sf::Style::Default);

    sf::Sprite  sprite;
    sf::Texture image_texture;
    sf::Text    fps_counter;
    sf::Font    font;

    font.loadFromFile("Font.ttf");    
    fps_counter.setFont(font);
    fps_counter.setFillColor(sf::Color::White);
    fps_counter.setPosition(0, 0);

    InitTimer();

    while (window.isOpen())
    {
        StartTimer();
        AlphaBlendingV1(&result, &background, &foreground);
        //sf::Image result = AlphaBlendingV0(background, foreground);
        StopTimer();

        #ifdef DRAW
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (sf::Event::Closed)
                {
                    window.close();
                    break;
                }
            }
            float microsec = (float)GetTimerMicroseconds();
            if (microsec != 0)
                UpdateFpsViewer(&fps_counter, (1/microsec) * 1000000. * (double)kTimeCalcAlphaBlend);

            sf::Image res_image;
            res_image.create(result.width, result.height, sf::Color::White);

            for (int y = 0; y < result.height; y++)
            {
                for (int x = 0; x < result.width; x++)
                {
                    res_image.setPixel(x, result.height - y - 1, sf::Color(result.pixels[y * result.width + x].r, 
                                                                 result.pixels[y * result.width + x].g, 
                                                                 result.pixels[y * result.width + x].b));
                }
            }

            window.clear();
            image_texture.loadFromImage(res_image);
            sprite.setTexture(image_texture);

            window.draw(sprite);
            window.draw(fps_counter);
            window.display();
        #else
            printf("FPS = %g\n", ((1/(float)(GetTimerMicroseconds)) * 1000000));
        #endif
    }
}

sf::Image AlphaBlendingV0(sf::Image background, sf::Image foreground)
{
    sf::Vector2u fg_size = foreground.getSize();

    for (size_t y = 0; y < fg_size.y; y++)
    {
        for (size_t x = 0; x < fg_size.x; x++)
        {
            sf::Color res_pixel;
            for (size_t t = 0; t < kTimeCalcAlphaBlend; t++)
            {
                sf::Color fg = foreground.getPixel(x, y);
                sf::Color bg = background.getPixel(x, y);

                res_pixel = sf::Color((fg.r * fg.a + bg.r * (255 - fg.a)) >> 8,
                                      (fg.g * fg.a + bg.g * (255 - fg.a)) >> 8,
                                      (fg.b * fg.a + bg.b * (255 - fg.a)) >> 8);
            }
            background.setPixel(x, y, res_pixel);
        }
    }

    return background;
}

void AlphaBlendingV1(Image_t* result, const Image_t* background, const Image_t* foreground)
{
    for (size_t y = 0; y < foreground->height; y++)
    {
        for (size_t x = 0; x < foreground->width; x++)
        {
            Pixel_t res_pixel;
            for (size_t t = 0; t < kTimeCalcAlphaBlend; t++)
            {
                Pixel_t fg = foreground->pixels[y * foreground->width + x];
                Pixel_t bg = background->pixels[y * background->width + x];

                #ifdef DEBUG
                    if (fg.g || fg.b || fg.r)
                    {
                        printf("(%d, %d)\n", x, y);
                        printf("(r = %d, g = %d, b = %d, a = %d)\n", fg.r, fg.g, fg.b, fg.a);
                    }
                #endif

                res_pixel.r = (fg.r * fg.a + bg.r * (255 - fg.a)) >> 8;
                res_pixel.g = (fg.g * fg.a + bg.g * (255 - fg.a)) >> 8;
                res_pixel.b = (fg.b * fg.a + bg.b * (255 - fg.a)) >> 8;
                //res_pixel.r = fg.r;
                //res_pixel.g = fg.g;
                //res_pixel.b = fg.b;
            }
            result->pixels[y * background->width + x] = res_pixel;
        }
    }
}

static void GetImageFromBMP(Image_t* image, const char* file_path)
{
    int fd = open(file_path, O_RDONLY);

    char* pixel_array = (char*)mmap(NULL, GetTextSize(file_path), PROT_READ, MAP_SHARED, fd, 0);

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

        for (int i = 0; i < image_arr_size; i++)
        {
            image->pixels[i].r = pixel_array[pixels_start_offset + i*3 + 2];
            image->pixels[i].g = pixel_array[pixels_start_offset + i*3 + 1];
            image->pixels[i].b = pixel_array[pixels_start_offset + i*3];
            //memcpy(&image->pixels[i], &pixel_array[pixels_start_offset] + i * 3, (sizeof(Pixel_t) - 1));
            image->pixels[i].a = 0xFF;
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

static void UpdateFpsViewer(sf::Text *fps_counter, float fps)
{
    char fps_str[kMaxFpsStrLen] = {};
    sprintf(fps_str, "%g", fps);

    fps_counter->setString(fps_str);
}

static int GetTextSize(const char *file_name)
{
    struct stat buff = {};
    stat(file_name, &buff);
    return buff.st_size;
}