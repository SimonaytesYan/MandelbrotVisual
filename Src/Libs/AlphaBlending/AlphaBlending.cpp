#include <stdlib.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <math.h>
#include <SFML/Graphics.hpp>
#include <string.h>

#include "AlphaBlending.h"
#include "../AlphaBlendingAVX/AlphaBlendingAVX.h"
#include "../AlignedCalloc/AlignedCalloc.h"
#include "../Stopwatch.h"

//#define DEBUG

sf::Image AlphaBlendingV0(sf::Image background, sf::Image foreground)
{
    sf::Vector2u fg_size = foreground.getSize();

    for (size_t y = 0; y < fg_size.y; y++)
    {
        for (size_t x = 0; x < fg_size.x; x++)
        {
            sf::Color res_pixel;
            sf::Color fg = foreground.getPixel(x, y);
            sf::Color bg = background.getPixel(x, y);

            res_pixel = sf::Color((sf::Uint8)((fg.r * fg.a + bg.r * (255 - fg.a)) >> 8),
                                  (sf::Uint8)((fg.g * fg.a + bg.g * (255 - fg.a)) >> 8),
                                  (sf::Uint8)((fg.b * fg.a + bg.b * (255 - fg.a)) >> 8));
            background.setPixel((unsigned int)x, (unsigned int)y, res_pixel);
        }
    }

    return background;
}

//!--------------------------------------------------------------------------------------
//!
//!Function copy background to result and then blend background image and foreground image
//!Background width and height must be not less then foreground width adn height
//!@param[out] result     blended background and foreground image 
//!@param[in]  backdround background image
//!@param[in]  foreground foreground image
//!
//!--------------------------------------------------------------------------------------
void AlphaBlendingV1(Image_t* result, const Image_t* background, const Image_t* foreground, const size_t kTimeCalcAlphaBlend)
{
    if (background->info.h < foreground->info.h || background->info.w < foreground->info.w)
    {
        fprintf(stderr, "Background is smaller than foreground\n");
        result = nullptr;
        return;
    }

    result->pixels = (Pixel_t*)realloc(result->pixels, sizeof(Pixel_t) * background->info.h * background->info.w);
    result->info.h = background->info.h;
    result->info.w = background->info.w; 
    memcpy(result->pixels, background->pixels, sizeof(Pixel_t) * background->info.h * background->info.w);

    static size_t numbr_meas = 0;
    static double sum_time   = 0;
    for (size_t t = 0; t < kTimeCalcAlphaBlend; t++)
    {
        clock_t startTime = clock();
        for (size_t y = 0; y < foreground->info.h; y++)
        {
            for (size_t x = 0; x < foreground->info.w; x++)
            {
                
                Pixel_t res_pixel;
                Pixel_t fg = foreground->pixels[y * foreground->info.w + x];
                Pixel_t bg = background->pixels[y * background->info.w + x];

                res_pixel.r = (char)((fg.r * fg.a + bg.r * (255 - fg.a)) >> 8);
                res_pixel.g = (char)((fg.g * fg.a + bg.g * (255 - fg.a)) >> 8);
                res_pixel.b = (char)((fg.b * fg.a + bg.b * (255 - fg.a)) >> 8);

                result->pixels[y * background->info.w + x] = res_pixel;
            }
            
        }

        numbr_meas++;
        double last_time = clock() - startTime;

        sum_time += last_time;
        if (numbr_meas % 50 == 0)
        {
            printf("av = %lf\n", sum_time / numbr_meas);
            sum_time   = 0;
            numbr_meas = 0;
        }
    }
}