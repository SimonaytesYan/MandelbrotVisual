#include <stdlib.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <math.h>

#include "DrawMandelbrot.h"
#include "../Stopwatch.h"

#define DRAW
//#define DEBUG

//==============================TESTING CONSTS==================================
const size_t kTimeCalcMandelbrotSet = 1;

//==============================WINDOW CONSTS===================================
const size_t kMaxFpsStrLen   = 20;
const char   kWindowHeader[] = "Mandelbrot set";

//==============================OTHER CONSTS====================================
const float   kMovingSpeed = 0.1;
const float   kZoomSpeed   = 0.05;
const float   _3210[4]     = {0, 1, 2, 3};
const __m512  _151413      = _mm512_set_ps(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
const __m512i _1_m512      = _mm512_set1_epi32(1);

//==============================FUNCTIONS PROTOTIPE===============================
static        sf::Color StepToColor(size_t step);
static        void      ProcessSetMoving(MandelbrotParams *params);
static        void      UpdateFpsViewer(sf::Text *fps_counter, float fps);
static inline float     GetDelta(float to, float from, size_t steps);

//==============================FOR WRAPERS======================================
static inline void mm_set_ps1(float* A, const float  elem) {for(int i = 0; i < 4; i++) A[i] = elem;}
static inline void mm_mul_ps (float* A, const float* B)    {for(int i = 0; i < 4; i++) A[i] = A[i] * B[i];}
static inline void mm_add_ps (float* A, const float* B)    {for(int i = 0; i < 4; i++) A[i] = A[i] + B[i];}
static inline void mm_sub_ps (float* A, const float* B)    {for(int i = 0; i < 4; i++) A[i] = A[i] - B[i];}
static inline void mm_mov_ps (float* A, const float* B)    {for(int i = 0; i < 4; i++) A[i] = B[i];}

void DrawMandelbrotSet(MandelbrotParams params)
{
    sf::RenderWindow window(sf::VideoMode(params.image_width, params.image_height), kWindowHeader, sf::Style::Default);

    sf::Image image;
    image.create(params.image_width, params.image_height, sf::Color::Black);

    sf::Texture image_texture;
    sf::Sprite  drawble;
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
        //ConstructMandelbrotV1(&image, &params);
        ConstructMandelbrotAVX512(&image, &params);
        StopTimer();

        #ifdef DRAW
            sf::Event event;
            while (window.pollEvent(event))
            {
                switch (event.type)
                {
                    case sf::Event::Closed:
                    {
                        window.close();
                        break;
                    }
                    case sf::Event::KeyPressed:
                    {
                       ProcessSetMoving(&params);
                       break;
                    }

                    default:
                        break;
                }
            }
            UpdateFpsViewer(&fps_counter, ((1/(float)(GetTimerMicroseconds())) * 1000000. * (double)kTimeCalcMandelbrotSet));

            window.clear();
            image_texture.loadFromImage(image);
            drawble.setTexture(image_texture);

            window.draw(drawble);
            window.draw(fps_counter);
            window.display();
        #else
            printf("FPS = %g\n", ((1/(float)(Get_timer_microseconds)) * 1000000));
        #endif
    }
}

//======================VERSION 1=====================
void ConstructMandelbrotV1(sf::Image* image, MandelbrotParams* params)
{
    const float kDeltaX = GetDelta(params->set_border.LeftBoder,   params->set_border.RightBoder, params->image_width);
    const float kDeltaY = GetDelta(params->set_border.BottomBoder, params->set_border.UpBoder,    params->image_height);

    float x0 = params->set_border.LeftBoder;
    for (size_t pixel_x = 0; pixel_x < params->image_width; pixel_x++, x0 += kDeltaX)
    {
        float y0 = params->set_border.BottomBoder;
        for(size_t pixel_y = 0; pixel_y < params->image_height; pixel_y++, y0 += kDeltaY)
        {
            bool draw_pixel = true;
            int i = 0;
            for(int T = 0; T < kTimeCalcMandelbrotSet; T++)
            {
                float X = x0;
                float Y = y0;

                draw_pixel = true;

                i = 0;
                for (i = 0; i < params->iterations; i++)
                {
                    if (params->radius_2 < X*X + Y*Y)
                    {
                        draw_pixel = false;
                        break;
                    }

                    float new_x = X*X - Y*Y + x0;
                    float new_y = 2*X*Y + y0;
                    X = new_x;
                    Y = new_y;
                }
            }

            if (!draw_pixel)
                image->setPixel(pixel_x, pixel_y, StepToColor(i));
            else 
                image->setPixel(pixel_x, pixel_y, sf::Color::Black);
            
        }
    }
}

//======================VERSION 2=====================
void ConstructMandelbrotV2(sf::Image* image, MandelbrotParams* params)
{
    const float  kDeltaX = GetDelta(params->set_border.LeftBoder,   params->set_border.RightBoder, params->image_width);
    const float  kDeltaY = GetDelta(params->set_border.BottomBoder, params->set_border.UpBoder,    params->image_height);

    float y0 = params->set_border.BottomBoder;
    for(size_t pixel_y = 0; pixel_y < params->image_height; pixel_y++, y0 += kDeltaY)
    {
        float x0 = params->set_border.LeftBoder;
        for (size_t pixel_x = 0; pixel_x < params->image_width; pixel_x+=4, x0 += 4*kDeltaX)
        {
            int draw_pixel = 0b1111;
            int steps[4]   = {};
            for(int T = 0; T < kTimeCalcMandelbrotSet; T++)
            {
                float X0[4] = {x0, x0 + kDeltaX, x0 + kDeltaX*2, x0 + kDeltaX*3};
                float Y0[4] = {y0, y0, y0, y0};
                float X[4]  = {}; for(int i = 0; i < 4; i++) X[i] = X0[i];
                float Y[4]  = {y0, y0, y0, y0};

                draw_pixel = 0b1111;
                
                for (int i = 0; i < 4; i++)
                    steps[i] = 0;
                for (int n = 0; n < params->iterations; n++)
                {
                    float XX[4] = {};   for (int i = 0; i < 4; i++) XX[i] = X[i]*X[i];
                    float YY[4] = {};   for (int i = 0; i < 4; i++) YY[i] = Y[i]*Y[i];
                    float XY[4] = {};   for (int i = 0; i < 4; i++) XY[i] = X[i]*Y[i];

                    for(int i = 0; i < 4; i++)
                    {
                        if ((draw_pixel & (1 << i)) && params->radius_2 < XX[i] + YY[i])
                        {
                            steps[i] = n;
                            draw_pixel |= 1 << i;
                        }
                    }
                    if (!draw_pixel)
                        break;

                    for (int i = 0; i < 4; i++) X[i] = XX[i] - YY[i] + X0[i];
                    for (int i = 0; i < 4; i++) Y[i] = 2*XY[i] + Y0[i];
                }
            }

            for (int i = 0; i < 4; i++)
            {
                if (draw_pixel & (1 << i))
                    image->setPixel(pixel_x + i, pixel_y, StepToColor(steps[i]));
            }
            
        }
    }
}

//======================VERSION 3=====================
void ConstructMandelbrotV3(sf::Image* image, MandelbrotParams* params)
{
    const float  kDeltaX = GetDelta(params->set_border.LeftBoder,   params->set_border.RightBoder, params->image_width);
    const float  kDeltaY = GetDelta(params->set_border.BottomBoder, params->set_border.UpBoder,    params->image_height);

    float y0 = params->set_border.BottomBoder;
    for(size_t pixel_y = 0; pixel_y < params->image_height; pixel_y++, y0 += kDeltaY)
    {
        float x0 = params->set_border.LeftBoder;
        for (size_t pixel_x = 0; pixel_x < params->image_width; pixel_x+=4, x0 += 4*kDeltaX)
        {
            int draw_pixel = 0b1111;
            int N[4]       = {};
            for(int T = 0; T < kTimeCalcMandelbrotSet; T++)
            {
                float X0[4] = {};
                mm_set_ps1(X0, kDeltaX);
                mm_mul_ps(X0, _3210);
                float X[4] = {};
                mm_set_ps1(X, x0);
                mm_add_ps(X0, X);
    
                mm_mov_ps(X, X0);
    
                float Y0[4] = {};
                mm_set_ps1(Y0, y0);
                float Y[4]  = {};
                mm_set_ps1(Y, y0);
                    
                draw_pixel = 0b1111;
    
                for (int i = 0; i < 4; i++)
                    N[i] = 0;

                int n = 0;
                for (n = 0; n < params->iterations; n++)
                {
                    float XX[4] = {};
                    mm_mov_ps(XX, X);
                    mm_mul_ps(XX, X);
                    float YY[4] = {};
                    mm_mov_ps(YY, Y);
                    mm_mul_ps(YY, Y);
                    float XY[4] = {};
                    mm_mov_ps(XY, X);
                    mm_mul_ps(XY, Y);
    
                    for(int i = 0; i < 4; i++)
                    {
                        if ((draw_pixel & (1 << i)) && params->radius_2 < XX[i] + YY[i])
                        {
                            N[i] = n;
                            draw_pixel |= 1 << i;
                        }
                    }
                    if (!draw_pixel)
                        break;
                        
                    mm_mov_ps(X, X0);
                    mm_add_ps(X, XX);
                    mm_sub_ps(X, YY);
    
                    mm_mov_ps(Y, Y0);
                    mm_add_ps(Y, XY);
                    mm_add_ps(Y, XY);
                }
            }

            for (int i = 0; i < 4; i++)
            {
                if (draw_pixel & (1 << i))
                    image->setPixel(pixel_x + i, pixel_y, StepToColor(N[i]));
            }
                
        }
    }
}

//======================VERSION 4=====================
void ConstructMandelbrotSSE(sf::Image* image, MandelbrotParams* params)
{
    const float  kDeltaX = GetDelta(params->set_border.LeftBoder,   params->set_border.RightBoder, params->image_width);
    const float  kDeltaY = GetDelta(params->set_border.BottomBoder, params->set_border.UpBoder,    params->image_height);
    const __m128 radius_2m128 = _mm_set1_ps(params->radius_2);

    float y0 = params->set_border.BottomBoder;
    for(size_t pixel_y = 0; pixel_y < params->image_height; pixel_y++, y0 += kDeltaY)
    {
        float x0 = params->set_border.LeftBoder;
        for (size_t pixel_x = 0; pixel_x < params->image_width; pixel_x += 4, x0 += 4*kDeltaX)
        {
            __m128i steps = _mm_setzero_si128();
            for(int T = 0; T < kTimeCalcMandelbrotSet; T++)
            {
                __m128 X0 = _mm_set_ps(x0 + kDeltaX*3, x0 + kDeltaX*2, x0 + kDeltaX, x0);
                __m128 Y0 = _mm_set_ps1(y0);
                __m128 X  = X0;
                __m128 Y  = Y0;

                steps = _mm_setzero_si128();   

                for (int n = 0; n < params->iterations; n++)
                {
                    __m128 XX = _mm_mul_ps(X, X);
                    __m128 YY = _mm_mul_ps(Y, Y);
                    __m128 XY = _mm_mul_ps(X, Y);

                    __m128 radius_2 = _mm_add_ps(XX, YY);

                    __m128 cmp = _mm_cmpge_ps(radius_2m128, radius_2);
                    if (!_mm_movemask_ps(cmp))
                        break;

                    steps = _mm_sub_epi32(steps, _mm_castps_si128(cmp));

                    X = _mm_add_ps(_mm_sub_ps(XX, YY), X0);
                    Y = _mm_add_ps(_mm_add_ps(XY, XY), Y0);
                }
            }

            int* step_int = (int*)&steps;
            for (int i = 0; i < 4; i++)
            {
                if (step_int[i])
                    image->setPixel(pixel_x + i, pixel_y, StepToColor(step_int[i]));
            }           
        }
    }
}

//======================VERSION 5=====================
void ConstructMandelbrotAVX512(sf::Image* image, MandelbrotParams* params)
{
    const float  kDeltaX = GetDelta(params->set_border.LeftBoder,   params->set_border.RightBoder, params->image_width);
    const float  kDeltaY = GetDelta(params->set_border.BottomBoder, params->set_border.UpBoder,    params->image_height);
    const __m512 radius_2m512 = _mm512_set1_ps(params->radius_2);

    float y0 = params->set_border.BottomBoder;
    for(size_t pixel_y = 0; pixel_y < params->image_height; pixel_y++, y0 += kDeltaY)
    {
        float x0 = params->set_border.LeftBoder;
        for (size_t pixel_x = 0; pixel_x < params->image_width; pixel_x += 16, x0 += 16*kDeltaX)
        {
            __m512i steps = _mm512_setzero_si512();
            for(int T = 0; T < kTimeCalcMandelbrotSet; T++)
            {
                __m512 X0 = _mm512_add_ps(_mm512_set1_ps(x0), _mm512_mul_ps(_151413, _mm512_set1_ps(kDeltaX)));

                __m512 Y0 = _mm512_set1_ps(y0);
                __m512 X  = X0;
                __m512 Y  = Y0;

                #ifdef DEBUG
                    printf("%lg %lg\n", x0, y0);
                        int kdf = 0;
                        scanf("%c", &kdf);
                #endif

                steps = _mm512_setzero_si512();

                for (int n = 0; n < params->iterations; n++)
                {
                    __m512 XX = _mm512_mul_ps(X, X);
                    __m512 YY = _mm512_mul_ps(Y, Y);
                    __m512 XY = _mm512_mul_ps(X, Y);
    
                    __m512 radius_2 = _mm512_add_ps(XX, YY);
    
                    __mmask16 cmp = _mm512_cmp_ps_mask(radius_2, radius_2m512, _CMP_LE_OS);
                    
                    steps = _mm512_add_epi32(steps, _mm512_maskz_mov_epi32(cmp, _1_m512));
    
                    #ifdef DEBUG
                        for (int i = 0; i < 16; i++)
                        {
                            printf("x[i]y[i] = (%lg %lg)\n", ((float*)&X)[i], ((float*)&Y)[i]);
                            //printf("r[%d]    = %lg\n", i, ((float*)&radius_2)[i]);
                        }
                        printf("cmp = %x\n\n", cmp);
                    #endif
    
                    if (cmp == 0)
                        break;
                        
                    X = _mm512_add_ps(_mm512_sub_ps(XX, YY), X0);
                    Y = _mm512_add_ps(_mm512_add_ps(XY, XY), Y0);
                }
            }

            int* step_int = (int*)&steps;
            for (int i = 0; i < 16; i++)
            {
                #ifdef DEBUG
                    printf("step[%d] = %d\n", i, step_int[i]);
                #endif
                if (step_int[i])
                    image->setPixel(pixel_x + i, pixel_y, StepToColor(step_int[i]));
            }           
        }
    }
}

//=============================================OTHER FUNCTIONS=================================

static void UpdateFpsViewer(sf::Text *fps_counter, float fps)
{
    char fps_str[kMaxFpsStrLen] = {};
    sprintf(fps_str, "%g", fps);

    fps_counter->setString(fps_str);
}

static inline float GetDelta(float a, float b, size_t steps)
{
    return abs(a - b)/(float)steps;
}

static sf::Color StepToColor(size_t step)
{
    return sf::Color((step*7)%256, (step*3)%256, (step*5)%256);
}

static void ProcessSetMoving(MandelbrotParams *params)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        params->set_border.LeftBoder  -= kMovingSpeed*params->zoom_lvl;
        params->set_border.RightBoder -= kMovingSpeed*params->zoom_lvl;
    }
        
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        params->set_border.LeftBoder  += kMovingSpeed*params->zoom_lvl;
        params->set_border.RightBoder += kMovingSpeed*params->zoom_lvl;
    }
        
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        params->set_border.UpBoder     -= kMovingSpeed*params->zoom_lvl;
        params->set_border.BottomBoder -= kMovingSpeed*params->zoom_lvl;
    }
        
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        params->set_border.UpBoder     += kMovingSpeed*params->zoom_lvl;
        params->set_border.BottomBoder += kMovingSpeed*params->zoom_lvl;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
    {
        params->set_border.UpBoder     -= kZoomSpeed * sqrt(params->zoom_lvl);
        params->set_border.BottomBoder += kZoomSpeed * sqrt(params->zoom_lvl);
        params->set_border.RightBoder  -= kZoomSpeed * sqrt(params->zoom_lvl);
        params->set_border.LeftBoder   += kZoomSpeed * sqrt(params->zoom_lvl);
        params->zoom_lvl /= 1.065;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
    {
        params->set_border.UpBoder     += kZoomSpeed * sqrt(params->zoom_lvl);
        params->set_border.BottomBoder -= kZoomSpeed * sqrt(params->zoom_lvl);
        params->set_border.RightBoder  += kZoomSpeed * sqrt(params->zoom_lvl);
        params->set_border.LeftBoder   -= kZoomSpeed * sqrt(params->zoom_lvl);
        params->zoom_lvl *= 1.065;
    }
}