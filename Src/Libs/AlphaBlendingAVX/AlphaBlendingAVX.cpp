#include <assert.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "AlphaBlendingAVX.h"
#include "../AlignedCalloc/AlignedCalloc.h"
#include "../Stopwatch.h"
//#define DEBUG

//=================================================CONSTANTS=================================================
const __m512i k0To1And2To3 = _mm512_set_epi8(0x80, 63, 0x80, 61,
                                             0x80, 59, 0x80, 57,
                                             0x80, 55, 0x80, 53,
                                             0x80, 51, 0x80, 49,
                                             0x80, 47, 0x80, 45,
                                             0x80, 43, 0x80, 41,
                                             0x80, 39, 0x80, 37,
                                             0x80, 35, 0x80, 33,
                                             0x80, 31, 0x80, 29,
                                             0x80, 27, 0x80, 25,
                                             0x80, 23, 0x80, 21,
                                             0x80, 19, 0x80, 17,
                                             0x80, 15, 0x80, 13,
                                             0x80, 11, 0x80,  9,
                                             0x80,  7, 0x80,  5,
                                             0x80,  3, 0x80,  1);

const __m512i k0To1And0To3 = _mm512_set_epi8(0x80, 63, 0x80, 63,
                                             0x80, 59, 0x80, 59,
                                             0x80, 55, 0x80, 55,
                                             0x80, 51, 0x80, 51,
                                             0x80, 47, 0x80, 47,
                                             0x80, 43, 0x80, 43,
                                             0x80, 39, 0x80, 39,
                                             0x80, 35, 0x80, 35,
                                             0x80, 31, 0x80, 31,
                                             0x80, 27, 0x80, 27,
                                             0x80, 23, 0x80, 23,
                                             0x80, 19, 0x80, 19,
                                             0x80, 15, 0x80, 15,
                                             0x80, 11, 0x80, 11,
                                             0x80,  7, 0x80,  7,
                                             0x80,  3, 0x80,  3);

const __m512i kZeroEverySecond = _mm512_set_epi8(0xFF, 0x00, 0xFF, 0x00,
                                                  0xFF, 0x00, 0xFF, 0x00,
                                                  0xFF, 0x00, 0xFF, 0x00,
                                                  0xFF, 0x00, 0xFF, 0x00,
                                                  0xFF, 0x00, 0xFF, 0x00,
                                                  0xFF, 0x00, 0xFF, 0x00,
                                                  0xFF, 0x00, 0xFF, 0x00,
                                                  0xFF, 0x00, 0xFF, 0x00,
                                                  0xFF, 0x00, 0xFF, 0x00,
                                                  0xFF, 0x00, 0xFF, 0x00,
                                                  0xFF, 0x00, 0xFF, 0x00,
                                                  0xFF, 0x00, 0xFF, 0x00,
                                                  0xFF, 0x00, 0xFF, 0x00,
                                                  0xFF, 0x00, 0xFF, 0x00,
                                                  0xFF, 0x00, 0xFF, 0x00,
                                                  0xFF, 0x00, 0xFF, 0x00);

const __m512i _255 = _mm512_set1_epi16(255);

const unsigned long long kGetEverySecFromFirst = 0b0101010101010101010101010101010101010101010101010101010101010101;
const unsigned long long kGetEveryFirstFromSec = 0b1010101010101010101010101010101010101010101010101010101010101010;

//=============================================FUNCTIONS PROTOTIPE==========================================
static void Printfm512i(__m512i a);
static void CalcAlphaBlendedPixels   (Pixel_t* process_part, const Image_t* backgr, const Image_t* foregr);
static void CalcAlphaBlendedPixels_V1(Image_t* result,       const Image_t* backgr, const Image_t* foregr);
static void CalcAlphaBlendedPixels_V2(Image_t* result,       const Image_t* backgr, const Image_t* foregr);

//=============================================FUNCTIONS IMPLEMENTATIONS==========================================

//======================VERSION 0=====================
void CalcAlphaBlendedPixels(Pixel_t* process_part, const Image_t* backgr, const Image_t* foregr)
{
    size_t   fg_pixel_num    = foregr->info.h*foregr->info.w;
    for (size_t pixel = 0; pixel < fg_pixel_num; pixel += 16)               //Process alhpa blending
    {
        __m512i foregr16 = _mm512_load_si512(&foregr->pixels[pixel]);
        __m512i backgr16 = _mm512_load_si512(&process_part[pixel]);

        //----------------------------------------------------
        //fg    = [a0 r0 g0 b0][a1 r1 g1 b1][a2 r2 g2 b2]...
        //fg_rb = [00 r0 00 b0][00 r1 00 b1][00 r2 00 b2]...
        //----------------------------------------------------

        __m512i foregr16_rb = _mm512_maskz_mov_epi8(kGetEverySecFromFirst, foregr16);
        __m512i backgr16_rb = _mm512_maskz_mov_epi8(kGetEverySecFromFirst, backgr16);

        //----------------------------------------------------
        //fg    = [a0 r0 g0 b0][a1 r1 g1 b1][a2 r2 g2 b2]...
        //fg_ag = [00 a0 00 g0][00 a1 00 g1][00 a2 00 g2]...
        //----------------------------------------------------

        __m512i foregr16_ag = _mm512_shuffle_epi8(foregr16, k0To1And2To3);
        __m512i backgr16_ag = _mm512_shuffle_epi8(backgr16, k0To1And2To3);

        //----------------------------------------------------
        //fg   = [a0 r0 g0 b0][a1 r1 g1 b1][a2 r2 g2 b2]...
        //fg_a = [00 a0 00 a0][00 a1 00 a1][00 a2 00 a2]...
        //----------------------------------------------------

        __m512i foregr16_alpha = _mm512_shuffle_epi8(foregr16, k0To1And0To3);

        //----------------------------------------------------
        //fg_ag  = [00 a0 00 g0][00 a1 00 g1][00 a2 00 g2]...
        //fg_a   = [00 a0 a0 a0][00 a1 00 a1][00 a2 00 a2]...
        //
        //res_ag = [A0 ** G0 **][A1 ** G1 **][A2 ** G2 **]...
        //res_rb = [R0 ** B0 **][R1 ** B1 **][R2 ** B2 **]...
        //----------------------------------------------------

        __m512i _255_foregr16_alpha = _mm512_sub_epi16(_mm512_set1_epi16(255), foregr16_alpha);
        __m512i result_rb           = _mm512_add_epi8 (_mm512_mullo_epi16(foregr16_alpha, foregr16_rb), _mm512_mullo_epi16(_255_foregr16_alpha, backgr16_rb));
        __m512i result_ag           = _mm512_add_epi8 (_mm512_mullo_epi16(foregr16_alpha, foregr16_ag), _mm512_mullo_epi16(_255_foregr16_alpha, backgr16_ag));

        //-----------------------------------------------------
        //res_rb = [R0 ** B0 **][R1 ** B1 **][R2 ** B2 **]...
        //res    = [00 R0 00 B0][00 R1 00 B1][00 R2 00 B2]
        //-----------------------------------------------------

        __m512i result = _mm512_shuffle_epi8(result_rb, k0To1And2To3);

        //-----------------------------------------------------
        //res_ag = [A0 ** G0 **][A1 ** G1 **][A2 ** G2 **]...
        //res    = [A0 R0 G0 B0][A1 R1 G1 B1][A2 R2 G2 B2]
        //-----------------------------------------------------

        result = _mm512_or_epi32(result, _mm512_and_epi32(result_ag, kZeroEverySecond));

        _mm512_store_si512((__m512i*)&process_part[pixel], result);     //put res in memory
    }  
}

//!--------------------------------------------------------------------------------------
//!
//!Function copy background to result and then blend background image and foreground image
//!Background width and height must be not less then foreground width adn height
//!@param[out] result     blended background and foreground image 
//!@param[in]  background background image
//!@param[in]  foreground foreground image
//!
//!--------------------------------------------------------------------------------------
void AlphaBlendingAVX512(Image_t* result, const Image_t* backgr, const Image_t* foregr, const size_t kTimeCalcAlphaBlend)
{
    if (backgr->info.h < foregr->info.h || backgr->info.w < foregr->info.w)
    {
        fprintf(stderr, "Background is smaller than foreground\n");
        result = nullptr;
        return;
    }

    size_t bg_size = sizeof(Pixel_t) * backgr->info.h * backgr->info.w;
    
    result->pixels = (Pixel_t*)realloc(result->pixels, bg_size);             //
    result->info.h = backgr->info.h;                                         //
    result->info.w  = backgr->info.w;                                        //Copy background to result
    memcpy(result->pixels, backgr->pixels, bg_size);                         //

    //---------------------------------------------
    
    assert(kGetEverySecFromFirst == 0x5555555555555555);
    for (size_t t = 0; t < kTimeCalcAlphaBlend; t++)
    {   
        size_t   fg_pixel_num    = foregr->info.h*foregr->info.w;
        Pixel_t* na_process_part = nullptr;                       //not aligned pointer to processing part
        Pixel_t* process_part    = (Pixel_t*)AlignedCalloc((void**)&na_process_part, 
                                                            sizeof(Pixel_t)*fg_pixel_num, 64);

        size_t line_len = sizeof(Pixel_t) * foregr->info.w;   
        for (size_t y = 0; y < foregr->info.h; y++)              //put processing part of background in one-dimensional array
            memcpy(process_part + y*foregr->info.w, &backgr->pixels[y * backgr->info.w], line_len);

        CalcAlphaBlendedPixels(process_part, backgr, foregr);

        for (size_t y = 0; y < foregr->info.h; y++)          //put processing part to result
            memcpy(&result->pixels[y*backgr->info.w], process_part + y*foregr->info.w, line_len);

        AlignedFree((void*)na_process_part);
    }
}

//======================VERSION 1=====================
static void CalcAlphaBlendedPixels_V1(Image_t* result, const Image_t* backgr, const Image_t* foregr)
{
    for (size_t y = 0; y < foregr->info.h; y++)
    {
        for (size_t x = 0; x < foregr->info.w; x += 16)               //Process alpha blending
        {
            __m512i foregr16 = _mm512_load_si512(&foregr->pixels[y*foregr->info.w + x]);
            __m512i backgr16 = _mm512_load_si512(&backgr->pixels[y*backgr->info.w + x]);

            __m512i foregr16_ag = _mm512_shuffle_epi8(foregr16, k0To1And2To3);
            __m512i backgr16_ag = _mm512_shuffle_epi8(backgr16, k0To1And2To3);
            __m512i foregr16_a  = _mm512_shuffle_epi8(foregr16, k0To1And0To3);
            
            __m512i foregr16_rb = _mm512_maskz_mov_epi8(kGetEverySecFromFirst, foregr16);
            __m512i backgr16_rb = _mm512_maskz_mov_epi8(kGetEverySecFromFirst, backgr16);

            __m512i _255_fg_a = _mm512_sub_epi16(_mm512_set1_epi16(255), foregr16_a);
            __m512i res_rb    = _mm512_add_epi8(_mm512_mullo_epi16(foregr16_a, foregr16_rb), _mm512_mullo_epi16(_255_fg_a, backgr16_rb));
            __m512i res_ag    = _mm512_add_epi8(_mm512_mullo_epi16(foregr16_a, foregr16_ag), _mm512_mullo_epi16(_255_fg_a, backgr16_ag));

            __m512i res = _mm512_shuffle_epi8(res_rb, k0To1And2To3);

            res = _mm512_or_epi32(res, _mm512_and_epi32(res_ag, kZeroEverySecond));
            _mm512_storeu_si512((__m512i*)&result->pixels[y*backgr->info.w + x], res);     //put res in memory
        }
    }
}

//!--------------------------------------------------------------------------------------
//!
//!Function copy background to result and then blend background image and foreground image
//!Background width and height must be not less then foreground width adn height
//!@param[out] result     blended background and foreground image 
//!@param[in]  background background image
//!@param[in]  foreground foreground image
//!
//!--------------------------------------------------------------------------------------
void AlphaBlendingAVX512_V1(Image_t* result, const Image_t* backgr, const Image_t* foregr, const size_t kTimeCalcAlphaBlend)
{
    if (backgr->info.h < foregr->info.h || backgr->info.w < foregr->info.w)
    {
        fprintf(stderr, "Background is smaller than foreground\n");
        result = nullptr;
        return;
    }

    size_t bg_size = sizeof(Pixel_t) * backgr->info.h * backgr->info.w;
    
    result->pixels = (Pixel_t*)realloc(result->pixels, bg_size);             //
    result->info.h = backgr->info.h;                                         //
    result->info.w  = backgr->info.w;                                        //Copy background to result
    memcpy(result->pixels, backgr->pixels, bg_size);                         //

    assert(kGetEverySecFromFirst == 0x5555555555555555);

    for (size_t t = 0; t < kTimeCalcAlphaBlend; t++)
    {
        CalcAlphaBlendedPixels_V1(result, backgr, foregr);
    }
}

//======================VERSION 2=====================
inline static void CalcAlphaBlendedPixels_V2(Image_t* result, const Image_t* backgr, const Image_t* foregr)
{
    static size_t numbr_meas = 0;
    static double sum_time   = 0;
    clock_t startTime = clock();

    size_t ind_bg = 0;
    size_t ind_fg = 0;
    size_t images_size =  foregr->info.h * foregr->info.w;
    for (size_t pixel_num = 0; pixel_num < images_size - 15; pixel_num += 16)
    {
        __m512i foregr16    = _mm512_load_si512(&(foregr->pixels[pixel_num]));
        __m512i backgr16    = _mm512_load_si512(&(backgr->pixels[pixel_num]));

        __m512i foregr16_ag = _mm512_shuffle_epi8(foregr16, k0To1And2To3);
        __m512i backgr16_ag = _mm512_shuffle_epi8(backgr16, k0To1And2To3);
        __m512i foregr16_a  = _mm512_shuffle_epi8(foregr16, k0To1And0To3);
                
        foregr16 = _mm512_maskz_mov_epi8(kGetEverySecFromFirst, foregr16);       //put in fg fg_rb
        backgr16 = _mm512_maskz_mov_epi8(kGetEverySecFromFirst, backgr16);       //put in bg bg_rb

        foregr16    = _mm512_mullo_epi16(foregr16_a, foregr16);
        foregr16_ag = _mm512_mullo_epi16(foregr16_a, foregr16_ag);

        foregr16_a =  _mm512_sub_epi16(_255, foregr16_a);

        backgr16    = _mm512_mullo_epi16(foregr16_a, backgr16);
        backgr16_ag = _mm512_mullo_epi16(foregr16_a, backgr16_ag);

        backgr16    = _mm512_add_epi8(foregr16,    backgr16);
        backgr16_ag = _mm512_add_epi8(foregr16_ag, backgr16_ag);

        backgr16 = _mm512_shuffle_epi8(backgr16, k0To1And2To3);

        backgr16 = _mm512_or_epi32(backgr16, _mm512_and_epi32(backgr16_ag, kZeroEverySecond));

        _mm512_store_si512((__m512i*)&result->pixels[pixel_num], backgr16);     //put res in memory
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
    for (size_t pixel_num = images_size - 15; pixel_num < images_size; pixel_num ++)
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
void AlphaBlendingAVX512_V2(Image_t* result, const Image_t* backgr, const Image_t* foregr, const size_t kTimeCalcAlphaBlend)
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
    
    assert(kGetEverySecFromFirst == 0x5555555555555555);

    //InitTimer();
    //StartTimer();
    for (size_t t = 0; t < kTimeCalcAlphaBlend; t++)
    {
        CalcAlphaBlendedPixels_V2(result, backgr, foregr);
    }

    //StopTimer();
    //double last_fps = 1/(double)GetTimerMicroseconds()* 1000. * (double)kTimeCalcAlphaBlend;
    //printf("last = %lf\n", last_fps);
}

static void Printfm512i(__m512i a)
{
    unsigned int* part_a = (unsigned int*)(&a);
    for (int i = 0; i < 16; i++)
        printf("[%08x]", part_a[i]);
}