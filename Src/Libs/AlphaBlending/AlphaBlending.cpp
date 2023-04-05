#include <stdlib.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <math.h>
#include <SFML/Graphics.hpp>
#include <string.h>

#include "AlphaBlending.h"
#include "../Stopwatch.h"

#define DRAW
//#define DEBUG

const size_t kMaxFpsStrLen = 20;

const char   kWindowHeader[]     = "Alpha blending";
const size_t kTimeCalcAlphaBlend = 1;

static void UpdateFpsViewer(sf::Text *fps_counter, float fps);
void AlphaBlendingAVX512(Image_t* result, const Image_t* backgr, const Image_t* foregr);

void MakeAlphaBlending(const char* background_path, const  char* foreground_path)
{
    Image_t background = {};
    Image_t foreground = {};

    GetImageFromBMP(&background, background_path);
    GetImageFromBMP(&foreground, foreground_path);

    Image_t result = {};
    result.pixels  = (Pixel_t*)calloc(sizeof(Pixel_t), background.info.h * background.info.w);
    result.info.h       = background.info.h;
    result.info.w       = background.info.w;

    sf::RenderWindow window(sf::VideoMode(background.info.w, background.info.h), kWindowHeader, sf::Style::Default);

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
            res_image.create(result.info.w, result.info.h, sf::Color::White);

            for (int y = 0; y < result.info.h; y++)
            {
                for (int x = 0; x < result.info.w; x++)
                {
                    res_image.setPixel(x, result.info.h - y - 1, sf::Color(result.pixels[y * result.info.w + x].r, 
                                                                 result.pixels[y * result.info.w + x].g, 
                                                                 result.pixels[y * result.info.w + x].b));
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

                res_pixel = sf::Color((sf::Uint8)((fg.r * fg.a + bg.r * (255 - fg.a)) >> 8),
                                      (sf::Uint8)((fg.g * fg.a + bg.g * (255 - fg.a)) >> 8),
                                      (sf::Uint8)((fg.b * fg.a + bg.b * (255 - fg.a)) >> 8));
            }
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
void AlphaBlendingV1(Image_t* result, const Image_t* background, const Image_t* foreground)
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

    for (size_t y = 0; y < foreground->info.h; y++)
    {
        for (size_t x = 0; x < foreground->info.w; x++)
        {
            Pixel_t res_pixel;
            for (size_t t = 0; t < kTimeCalcAlphaBlend; t++)
            {
                Pixel_t fg = foreground->pixels[y * foreground->info.w + x];
                Pixel_t bg = background->pixels[y * background->info.w + x];

                res_pixel.r = (char)((fg.r * fg.a + bg.r * (255 - fg.a)) >> 8);
                res_pixel.g = (char)((fg.g * fg.a + bg.g * (255 - fg.a)) >> 8);
                res_pixel.b = (char)((fg.b * fg.a + bg.b * (255 - fg.a)) >> 8);

                #ifdef DEBUG
                    printf("(%d, %d)\n", x, y);
                    printf("fg  = (r = %x, g = %x, b = %x, a = %x)\n", fg.r, fg.g, fg.b, fg.a);
                    printf("res = (r = %x, g = %x, b = %x, a = %x)\n", res_pixel.r, res_pixel.g, res_pixel.b, res_pixel.a);
                    int k = 0;

                    scanf("%c", &k);
                #endif
            }
            result->pixels[y * background->info.w + x] = res_pixel;
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
void AlphaBlendingAVX512(Image_t* result, const Image_t* backgr, const Image_t* foregr)
{
    if (backgr->info.h < foregr->info.h || backgr->info.w < foregr->info.w)
    {
        fprintf(stderr, "Background is smaller than foreground\n");
        result = nullptr;
        return;
    }

    size_t bg_size = sizeof(Pixel_t) * backgr->info.h * backgr->info.w;
    result->pixels = (Pixel_t*)realloc(result->pixels, bg_size + 512); //
    result->info.h = backgr->info.h;                                             //
    result->info.w  = backgr->info.w;                                            //Copy background to result
    memcpy(result->pixels, backgr->pixels, bg_size);                   //

    size_t   fg_pixel_num = foregr->info.h*foregr->info.w;
    Pixel_t* process_part = (Pixel_t*)calloc(sizeof(Pixel_t), fg_pixel_num);

    size_t line_len = sizeof(Pixel_t) * foregr->info.w;   
    for (size_t y = 0; y < foregr->info.h; y++)                  //put processing part of background in one-dimensional array
    {
        memcpy(process_part + y*foregr->info.w, &backgr->pixels[y * backgr->info.w], line_len);
    }

    for (size_t pixel = 0; pixel < fg_pixel_num; pixel += 16)         //Process alhpa blending
    {
        __m512i fg = _mm512_load_epi32(&foregr[pixel]);
        __m512i bg = _mm512_load_epi32(&process_part[pixel]);

        //----------------------------------------------------
        //fg    = [a0 r0 g0 b0][a1 r1 g1 b1][a2 r2 g2 b2]...
        //fg_rb = [00 r0 00 b0][00 r1 00 b1][00 r2 00 b2]...
        //----------------------------------------------------

        __m512i fg_rb = _mm512_maskz_mov_epi32(0x00FF00FF, fg);
        __m512i bg_rb = _mm512_maskz_mov_epi32(0x00FF00FF, bg);

        //----------------------------------------------------
        //fg    = [a0 r0 g0 b0][a1 r1 g1 b1][a2 r2 g2 b2]...
        //fg_ag = [00 a0 00 g0][00 a1 00 g1][00 a2 00 g2]...
        //----------------------------------------------------

        __m512i shuffle_mask1 = _mm512_set_epi8(0x80, 0x00, 0x80, 0x02,
                                                0x80, 0x04, 0x80, 0x06,
                                                0x80, 0x08, 0x80, 0x10,
                                                0x80, 0x12, 0x80, 0x14,
                                                0x80, 0x16, 0x80, 0x18,
                                                0x80, 0x20, 0x80, 0x22,
                                                0x80, 0x24, 0x80, 0x26,
                                                0x80, 0x28, 0x80, 0x30,
                                                0x80, 0x32, 0x80, 0x34,
                                                0x80, 0x36, 0x80, 0x38,
                                                0x80, 0x40, 0x80, 0x42,
                                                0x80, 0x44, 0x80, 0x46,
                                                0x80, 0x48, 0x80, 0x50,
                                                0x80, 0x52, 0x80, 0x54,
                                                0x80, 0x56, 0x80, 0x58,
                                                0x80, 0x60, 0x80, 0x62);

        __m512i fg_ag = _mm512_shuffle_epi8(fg, shuffle_mask1);
        __m512i bg_ag = _mm512_shuffle_epi8(bg, shuffle_mask1);

        //----------------------------------------------------
        //fg   = [a0 r0 g0 b0][a1 r1 g1 b1][a2 r2 g2 b2]...
        //fg_a = [00 a0 00 a0][00 a1 00 a1][00 a2 00 a2]...
        //----------------------------------------------------

        __m512i shuffle_mask2 = _mm512_set_epi8(0x80, 0x00, 0x80, 0x00,
                                                0x80, 0x04, 0x80, 0x04,
                                                0x80, 0x08, 0x80, 0x08,
                                                0x80, 0x12, 0x80, 0x12,
                                                0x80, 0x16, 0x80, 0x16,
                                                0x80, 0x20, 0x80, 0x20,
                                                0x80, 0x24, 0x80, 0x24,
                                                0x80, 0x28, 0x80, 0x28,
                                                0x80, 0x32, 0x80, 0x32,
                                                0x80, 0x36, 0x80, 0x36,
                                                0x80, 0x40, 0x80, 0x40,
                                                0x80, 0x44, 0x80, 0x44,
                                                0x80, 0x48, 0x80, 0x48,
                                                0x80, 0x52, 0x80, 0x52,
                                                0x80, 0x56, 0x80, 0x56,
                                                0x80, 0x60, 0x80, 0x60);

        __m512i fg_a = _mm512_shuffle_epi8(fg, shuffle_mask2);

        //----------------------------------------------------
        //fg_ag  = [00 a0 00 g0][00 a1 00 g1][00 a2 00 g2]...
        //fg_a   = [00 a0 a0 a0][00 a1 00 a1][00 a2 00 a2]...
        //
        //res_ag = [A0 ** G0 **][A1 ** G1 **][A2 ** G2 **]...
        //res_rb = [R0 ** B0 **][R1 ** B1 **][R2 ** B2 **]...
        //----------------------------------------------------

        __m512i res_rb = _mm512_mullo_epi16(fg_a, fg_rb) + _mm512_mullo_epi16(_mm512_sub_epi16(_mm512_set1_epi16(255), fg_a), bg_rb);
        __m512i res_ag = _mm512_mullo_epi16(fg_a, fg_ag) + _mm512_mullo_epi16(_mm512_sub_epi16(_mm512_set1_epi16(255), fg_a), bg_ag);

        //-----------------------------------------------------
        //res_rb = [R0 ** B0 **][R1 ** B1 **][R2 ** B2 **]...
        //res    = [00 R0 00 B0][00 R1 00 B1][00 R2 00 B2]
        //-----------------------------------------------------

        __m512i res = _mm512_shuffle_epi8(res_rb, shuffle_mask1);

        __m512i mask = _mm512_set_epi8(0xFF, 0x00, 0xFF, 0x00,
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

        res = _mm512_or_epi32(res, _mm512_and_epi32(res_ag, mask));
    }    
}

static void UpdateFpsViewer(sf::Text *fps_counter, float fps)
{
    char fps_str[kMaxFpsStrLen] = {};
    sprintf(fps_str, "%g", fps);

    fps_counter->setString(fps_str);
}