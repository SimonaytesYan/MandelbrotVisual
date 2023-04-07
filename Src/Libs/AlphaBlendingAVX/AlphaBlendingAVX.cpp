#include <stdlib.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <math.h>
#include <SFML/Graphics.hpp>
#include <string.h>

#include "AlphaBlendingAVX.h"
#include "../AlignedCalloc/AlignedCalloc.h"
#include "../Stopwatch.h"

#define DRAW
//#define DEBUG

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

static void Printfm512i(__m512i a);

//!--------------------------------------------------------------------------------------
//!
//!Function copy background to result and then blend background image and foreground image
//!Background width and height must be not less then foreground width adn height
//!@param[out] result     blended background and foreground image 
//!@param[in]  backdround background image
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


    for (size_t t = 0; t < kTimeCalcAlphaBlend; t++)
    {
        size_t   fg_pixel_num    = foregr->info.h*foregr->info.w;
        Pixel_t* na_process_part = nullptr;                                      //not aligned pointer to processing part
        Pixel_t* process_part    = (Pixel_t*)AlignedCalloc((void**)&na_process_part, 
                                                            sizeof(Pixel_t)*fg_pixel_num, 64);

        size_t line_len = sizeof(Pixel_t) * foregr->info.w;   
        for (size_t y = 0; y < foregr->info.h; y++)                             //put processing part of background in one-dimensional array
            memcpy(process_part + y*foregr->info.w, &backgr->pixels[y * backgr->info.w], line_len);

        for (size_t pixel = 0; pixel < fg_pixel_num; pixel += 16)               //Process alhpa blending
        {
            __m512i fg = _mm512_load_si512(&foregr->pixels[pixel]);
            __m512i bg = _mm512_load_si512(&process_part[pixel]);

            //----------------------------------------------------
            //fg    = [a0 r0 g0 b0][a1 r1 g1 b1][a2 r2 g2 b2]...
            //fg_rb = [00 r0 00 b0][00 r1 00 b1][00 r2 00 b2]...
            //----------------------------------------------------

            unsigned long long mask = 0b0101010101010101010101010101010101010101010101010101010101010101;
            __m512i fg_rb = _mm512_maskz_mov_epi8(mask, fg);
            __m512i bg_rb = _mm512_maskz_mov_epi8(mask, bg);

            //----------------------------------------------------
            //fg    = [a0 r0 g0 b0][a1 r1 g1 b1][a2 r2 g2 b2]...
            //fg_ag = [00 a0 00 g0][00 a1 00 g1][00 a2 00 g2]...
            //----------------------------------------------------

            __m512i fg_ag = _mm512_shuffle_epi8(fg, k0To1And2To3);
            __m512i bg_ag = _mm512_shuffle_epi8(bg, k0To1And2To3);

            //----------------------------------------------------
            //fg   = [a0 r0 g0 b0][a1 r1 g1 b1][a2 r2 g2 b2]...
            //fg_a = [00 a0 00 a0][00 a1 00 a1][00 a2 00 a2]...
            //----------------------------------------------------

            __m512i fg_a = _mm512_shuffle_epi8(fg, k0To1And0To3);

            //----------------------------------------------------
            //fg_ag  = [00 a0 00 g0][00 a1 00 g1][00 a2 00 g2]...
            //fg_a   = [00 a0 a0 a0][00 a1 00 a1][00 a2 00 a2]...
            //
            //res_ag = [A0 ** G0 **][A1 ** G1 **][A2 ** G2 **]...
            //res_rb = [R0 ** B0 **][R1 ** B1 **][R2 ** B2 **]...
            //----------------------------------------------------

            __m512i _255_fg_a = _mm512_sub_epi16(_mm512_set1_epi16(255), fg_a);
            __m512i res_rb    = _mm512_add_epi8(_mm512_mullo_epi16(fg_a, fg_rb), _mm512_mullo_epi16(_255_fg_a, bg_rb));
            __m512i res_ag    = _mm512_add_epi8(_mm512_mullo_epi16(fg_a, fg_ag), _mm512_mullo_epi16(_255_fg_a, bg_ag));

            //-----------------------------------------------------
            //res_rb = [R0 ** B0 **][R1 ** B1 **][R2 ** B2 **]...
            //res    = [00 R0 00 B0][00 R1 00 B1][00 R2 00 B2]
            //-----------------------------------------------------

            __m512i res = _mm512_shuffle_epi8(res_rb, k0To1And2To3);

            //-----------------------------------------------------
            //res_ag = [A0 ** G0 **][A1 ** G1 **][A2 ** G2 **]...
            //res    = [A0 R0 G0 B0][A1 R1 G1 B1][A2 R2 G2 B2]
            //-----------------------------------------------------

            res = _mm512_or_epi32(res, _mm512_and_epi32(res_ag, kZeroEverySecond));

            _mm512_store_si512((__m512i*)&process_part[pixel], res);     //put res in memory
        }    

        for (size_t y = 0; y < foregr->info.h; y++)          //put processing part to result
        {
            memcpy(&result->pixels[y*backgr->info.w], process_part + y*foregr->info.w, line_len);
        }

        AlignedFree((void*)na_process_part);
    }
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

    for (size_t t = 0; t < kTimeCalcAlphaBlend; t++)
    {
        for (size_t y = 0; y < foregr->info.h; y++)
        {
            for (size_t x = 0; x < foregr->info.w; x += 16)               //Process alhpa blending
            {
                __m512i fg = _mm512_loadu_si512(&foregr->pixels[y*foregr->info.w + x]);
                __m512i bg = _mm512_loadu_si512(&backgr->pixels[y*backgr->info.w + x]);

                const unsigned long long mask = 0b0101010101010101010101010101010101010101010101010101010101010101;
                __m512i fg_rb = _mm512_maskz_mov_epi8(mask, fg);
                __m512i bg_rb = _mm512_maskz_mov_epi8(mask, bg);

                __m512i fg_ag = _mm512_shuffle_epi8(fg, k0To1And2To3);
                __m512i bg_ag = _mm512_shuffle_epi8(bg, k0To1And2To3);

                __m512i fg_a = _mm512_shuffle_epi8(fg, k0To1And0To3);

                __m512i _255_fg_a = _mm512_sub_epi16(_mm512_set1_epi16(255), fg_a);
                __m512i res_rb    = _mm512_add_epi8(_mm512_mullo_epi16(fg_a, fg_rb), _mm512_mullo_epi16(_255_fg_a, bg_rb));
                __m512i res_ag    = _mm512_add_epi8(_mm512_mullo_epi16(fg_a, fg_ag), _mm512_mullo_epi16(_255_fg_a, bg_ag));

                __m512i res = _mm512_shuffle_epi8(res_rb, k0To1And2To3);

                res = _mm512_or_epi32(res, _mm512_and_epi32(res_ag, kZeroEverySecond));
                _mm512_storeu_si512((__m512i*)&result->pixels[y*backgr->info.w + x], res);     //put res in memory
            }
        }
    }
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
void AlphaBlendingAVX512_V2(Image_t* result, const Image_t* backgr, const Image_t* foregr, const size_t kTimeCalcAlphaBlend)
{
    if (backgr->info.h < foregr->info.h || backgr->info.w < foregr->info.w)
    {
        fprintf(stderr, "Background is smaller than foreground\n");
        return;
    }

    size_t bg_size = sizeof(Pixel_t) * backgr->info.h * backgr->info.w;
    
    result->pixels = (Pixel_t*)realloc(result->pixels, bg_size);             //
    result->info.h = backgr->info.h;                                         //
    result->info.w  = backgr->info.w;                                        //Copy background to result
    memcpy(result->pixels, backgr->pixels, bg_size);                         //

    for (size_t t = 0; t < kTimeCalcAlphaBlend; t++)
    {
        for (size_t y = 0; y < foregr->info.h; y++)
        {
            for (size_t x = 0; x < foregr->info.w; x += 16)               //Process alhpa blending
            {
                __m512i fg = _mm512_loadu_si512(&(foregr->pixels[y*foregr->info.w + x]));
                __m512i bg = _mm512_loadu_si512(&(backgr->pixels[y*backgr->info.w + x]));

                const unsigned long long mask = 0b0101010101010101010101010101010101010101010101010101010101010101;
                __m512i fg_rb = _mm512_maskz_mov_epi8(mask, fg);
                __m512i bg_rb = _mm512_maskz_mov_epi8(mask, bg);

                __m512i fg_a      = _mm512_shuffle_epi8(fg, k0To1And0To3);
                __m512i _255_fg_a = _mm512_sub_epi16(_mm512_set1_epi16(255), fg_a);

                __m512i res_rb    = _mm512_add_epi8(_mm512_mullo_epi16(fg_a, fg_rb), _mm512_mullo_epi16(_255_fg_a, bg_rb));

                __m512i fg_ag = _mm512_shuffle_epi8(fg, k0To1And2To3);
                __m512i bg_ag = _mm512_shuffle_epi8(bg, k0To1And2To3);

                __m512i res_ag = _mm512_add_epi8(_mm512_mullo_epi16(fg_a, fg_ag), _mm512_mullo_epi16(_255_fg_a, bg_ag));

                __m512i res = _mm512_shuffle_epi8(res_rb, k0To1And2To3);

                res = _mm512_or_epi32(res, _mm512_maskz_mov_epi8(~mask, res_ag));
                _mm512_storeu_si512((__m512i*)&result->pixels[y*backgr->info.w + x], res);     //put res in memory
            }
        }
    }
}


static void Printfm512i(__m512i a)
{
    unsigned int* part_a = (unsigned int*)(&a);
    for (int i = 0; i < 16; i++)
        printf("[%08x]", part_a[i]);
    
    //printf("[ind] = %x\n", a);    
}