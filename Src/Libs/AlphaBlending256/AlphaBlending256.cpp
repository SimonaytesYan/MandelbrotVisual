#include <assert.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "AlphaBlending256.h"
#include "../AlignedCalloc/AlignedCalloc.h"
#include "../Stopwatch.h"

//=================================================CONSTANTS=================================================
const __m256i k0To1And2To3 = _mm256_set_epi8(0x80, 31, 0x80, 29,
                                             0x80, 27, 0x80, 25,
                                             0x80, 23, 0x80, 21,
                                             0x80, 19, 0x80, 17,
                                             0x80, 15, 0x80, 13,
                                             0x80, 11, 0x80,  9,
                                             0x80,  7, 0x80,  5,
                                             0x80,  3, 0x80,  1);

const __m256i k0To1And0To3 = _mm256_set_epi8(0x80, 31, 0x80, 31,
                                             0x80, 27, 0x80, 27,
                                             0x80, 23, 0x80, 23,
                                             0x80, 19, 0x80, 19,
                                             0x80, 15, 0x80, 15,
                                             0x80, 11, 0x80, 11,
                                             0x80,  7, 0x80,  7,
                                             0x80,  3, 0x80,  3);

const __m256i kZeroEverySecond = _mm256_set_epi8(0xFF, 0x00, 0xFF, 0x00,
                                                0xFF, 0x00, 0xFF, 0x00,
                                                0xFF, 0x00, 0xFF, 0x00,
                                                0xFF, 0x00, 0xFF, 0x00,
                                                0xFF, 0x00, 0xFF, 0x00,
                                                0xFF, 0x00, 0xFF, 0x00,
                                                0xFF, 0x00, 0xFF, 0x00,
                                                0xFF, 0x00, 0xFF, 0x00);

const __m256i kGetEverySecFromFirst_256 = _mm256_set1_epi32(0xFF00FF00);
const __m256i _255                      = _mm256_set1_epi16(255);
const unsigned long long kGetEverySecFromFirst = 0b01010101010101010101010101010101;

//======================VERSION 2=====================
inline static void CalcAlphaBlendedPixels_V2(Image_t* result, const Image_t* backgr, const Image_t* foregr)
{
    static size_t numbr_meas = 0;
    static double sum_time   = 0;
    clock_t startTime = clock();
    
    size_t ind_bg = 0;
    size_t ind_fg = 0;
    size_t images_size =  foregr->info.h * foregr->info.w;
    for (size_t pixel_num = 0; pixel_num < images_size - 7; pixel_num += 8)
    {
        __m256i foregr16    = _mm256_load_si256((const __m256i*)&(foregr->pixels[pixel_num]));
        __m256i backgr16    = _mm256_load_si256((const __m256i*)&(backgr->pixels[pixel_num]));

        __m256i foregr16_ag = _mm256_shuffle_epi8(foregr16, k0To1And2To3);
        __m256i backgr16_ag = _mm256_shuffle_epi8(backgr16, k0To1And2To3);
        __m256i foregr16_a  = _mm256_shuffle_epi8(foregr16, k0To1And0To3);
                
        foregr16 = _mm256_and_si256(kGetEverySecFromFirst_256, foregr16);       //put in fg fg_rb
        backgr16 = _mm256_and_si256(kGetEverySecFromFirst_256, backgr16);       //put in bg bg_rbg

        foregr16    = _mm256_mullo_epi16(foregr16_a, foregr16);
        foregr16_ag = _mm256_mullo_epi16(foregr16_a, foregr16_ag);

        foregr16_a =  _mm256_sub_epi16(_255, foregr16_a);

        backgr16    = _mm256_mullo_epi16(foregr16_a, backgr16);
        backgr16_ag = _mm256_mullo_epi16(foregr16_a, backgr16_ag);

        backgr16    = _mm256_add_epi8(foregr16,    backgr16);
        backgr16_ag = _mm256_add_epi8(foregr16_ag, backgr16_ag);

        backgr16 = _mm256_shuffle_epi8(backgr16, k0To1And2To3);

        backgr16 = _mm256_or_epi32(backgr16, _mm256_and_si256(kGetEverySecFromFirst_256, backgr16_ag));

        _mm256_store_si256((__m256i*)&result->pixels[pixel_num], backgr16);     //put res in memory

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

    for (size_t pixel_num = images_size - 7; pixel_num < images_size; pixel_num ++)
    {
            Pixel_t res_pixel;
            Pixel_t fg = foregr->pixels[pixel_num];
            Pixel_t bg = backgr->pixels[pixel_num];

            res_pixel.r = (char)((fg.r * fg.a + bg.r * (255 - fg.a)) >> 8);
            res_pixel.g = (char)((fg.g * fg.a + bg.g * (255 - fg.a)) >> 8);
            res_pixel.b = (char)((fg.b * fg.a + bg.b * (255 - fg.a)) >> 8);

            result->pixels[pixel_num] = res_pixel;
    }
}

//!--------------------------------------------------------------------------------------
//!
//!Function copy background to result and then blend background image and foreground image
//!Background width and height must be the same size
//!@param[out] result     blended background and foreground image 
//!@param[in]  background background image
//!@param[in]  foreground foreground image
//!
//!--------------------------------------------------------------------------------------
void AlphaBlendingAVX256(Image_t* result, const Image_t* backgr, const Image_t* foregr, const size_t kTimeCalcAlphaBlend)
{
    if (backgr->info.h != foregr->info.h || backgr->info.w != foregr->info.w)
    {
        fprintf(stderr, "Background is smaller than foreground\n");
        return;
    }

    size_t bg_size = sizeof(Pixel_t) * backgr->info.h * backgr->info.w;
    
    free(result->real_array_ptr);
    ImageCtor(result, backgr->info.h, backgr->info.w, backgr->info.pixel_size, 64);
    memcpy(result->pixels, backgr->pixels, bg_size);
    
    assert(kGetEverySecFromFirst == 0x55555555);

    
    for (size_t t = 0; t < kTimeCalcAlphaBlend; t++)
    {
        CalcAlphaBlendedPixels_V2(result, backgr, foregr);
    }
}
