#include <emmintrin.h>
#include <immintrin.h>
#include <math.h>
#include <stdlib.h>

#include "CalcMandelbrot.h"
#include "../Stopwatch.h"

static inline float GetDelta(float a, float b, size_t steps)
{
    return abs(a - b)/(float)steps;
}

static sf::Color StepToColor(size_t step)
{
    if (step > 255)
        return sf::Color(255, 255 ,255);
    return sf::Color(0, 0, 0);
    // return sf::Color((step*7)%256, (step*3)%256, (step*5)%256);
}

//==============================LOOP WRAPPERS======================================
static inline void mm_set_ps1(float* A, const float  elem) {for(int i = 0; i < 4; i++) A[i] = elem;}
static inline void mm_mul_ps (float* A, const float* B)    {for(int i = 0; i < 4; i++) A[i] = A[i] * B[i];}
static inline void mm_add_ps (float* A, const float* B)    {for(int i = 0; i < 4; i++) A[i] = A[i] + B[i];}
static inline void mm_sub_ps (float* A, const float* B)    {for(int i = 0; i < 4; i++) A[i] = A[i] - B[i];}
static inline void mm_mov_ps (float* A, const float* B)    {for(int i = 0; i < 4; i++) A[i] = B[i];}


//======================VERSION 1=====================
void ConstructMandelbrotV1(sf::Image* image, MandelbrotParams* params)
{
    const float kDeltaX = GetDelta(params->set_border.LeftBorder,   params->set_border.RightBorder, params->image_width);
    const float kDeltaY = GetDelta(params->set_border.BottomBorder, params->set_border.UpBorder,    params->image_height);

    float x0 = params->set_border.LeftBorder;
    for (size_t pixel_x = 0; pixel_x < params->image_width; pixel_x++, x0 += kDeltaX)
    {
        float y0 = params->set_border.BottomBorder;
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
    const float  kDeltaX = GetDelta(params->set_border.LeftBorder,   params->set_border.RightBorder, params->image_width);
    const float  kDeltaY = GetDelta(params->set_border.BottomBorder, params->set_border.UpBorder,    params->image_height);

    float y0 = params->set_border.BottomBorder;
    for(size_t pixel_y = 0; pixel_y < params->image_height; pixel_y++, y0 += kDeltaY)
    {
        float x0 = params->set_border.LeftBorder;
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
    const float  _3210[4] = {0, 1, 2, 3};
    const float  kDeltaX  = GetDelta(params->set_border.LeftBorder,   params->set_border.RightBorder, params->image_width);
    const float  kDeltaY  = GetDelta(params->set_border.BottomBorder, params->set_border.UpBorder,    params->image_height);

    float y0 = params->set_border.BottomBorder;
    for(size_t pixel_y = 0; pixel_y < params->image_height; pixel_y++, y0 += kDeltaY)
    {
        float x0 = params->set_border.LeftBorder;
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

#ifdef AVX256
//======================VERSION 4=====================
void ConstructMandelbrotSSE(sf::Image* image, MandelbrotParams* params)
{
    const float  kDeltaX = GetDelta(params->set_border.LeftBorder,   params->set_border.RightBorder, params->image_width);
    const float  kDeltaY = GetDelta(params->set_border.BottomBorder, params->set_border.UpBorder,    params->image_height);
    const __m128 radius_2m128 = _mm_set1_ps(params->radius_2);

    float y0 = params->set_border.BottomBorder;
    for(size_t pixel_y = 0; pixel_y < params->image_height; pixel_y++, y0 += kDeltaY)
    {
        float x0 = params->set_border.LeftBorder;
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
#endif  // AVX256

#ifdef AVX512
const __m512  _151413      = _mm512_set_ps(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
const __m512i _1_m512      = _mm512_set1_epi32(1);

//======================VERSION 5=====================
void ConstructMandelbrotAVX512(sf::Image* image, MandelbrotParams* params)
{
    const float  kDeltaX = GetDelta(params->set_border.LeftBorder,   params->set_border.RightBorder, params->image_width);
    const float  kDeltaY = GetDelta(params->set_border.BottomBorder, params->set_border.UpBorder,    params->image_height);
    const __m512 radius_2m512 = _mm512_set1_ps(params->radius_2);

    float y0 = params->set_border.BottomBorder;
    for(size_t pixel_y = 0; pixel_y < params->image_height; pixel_y++, y0 += kDeltaY)
    {
        float x0 = params->set_border.LeftBorder;
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

typedef float v64f __attribute__((vector_size(64)));
typedef int v64i __attribute__((vector_size(64)));

#define VECTOR_I_CONSTANT(a) (v64i){a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a}
#define VECTOR_F_CONSTANT(a) (v64f){a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a}

//======================VERSION 6=====================
void ConstructMandelbrotAVX512UsefulFormat(sf::Image* image, MandelbrotParams* params)
{
    const float  kDeltaX = GetDelta(params->set_border.LeftBorder,   params->set_border.RightBorder, params->image_width);
    const float  kDeltaY = GetDelta(params->set_border.BottomBorder, params->set_border.UpBorder,    params->image_height);
    const v64f   radius_2m512 = VECTOR_F_CONSTANT(params->radius_2);
    const size_t param_number = sizeof(v64i) / sizeof(int);

    float y0 = params->set_border.BottomBorder;
    for(size_t pixel_y = 0; pixel_y < params->image_height; pixel_y++, y0 += kDeltaY)
    {
        float x0 = params->set_border.LeftBorder;
        for (size_t pixel_x = 0; pixel_x < params->image_width; pixel_x += param_number, x0 += param_number*kDeltaX)
        {
            v64i steps = {0};
            for(int T = 0; T < kTimeCalcMandelbrotSet; T++)
            {
                v64f X0 = VECTOR_F_CONSTANT(x0) + _151413 * VECTOR_F_CONSTANT(kDeltaX);

                v64f Y0 = VECTOR_F_CONSTANT(y0);
                v64f X  = X0;
                v64f Y  = Y0;

                steps = VECTOR_I_CONSTANT(0);

                for (int n = 0; n < params->iterations; n++)
                {
                    v64f XX = X * X;
                    v64f YY = Y * Y;
                    v64f XY = X * Y;
    
                    v64f radius_2 = XX + YY;

                    v64i cmp = (radius_2 < radius_2m512) * -1;
                    steps = steps + cmp;

                    if (_mm512_cmp_epi32_mask(cmp, VECTOR_I_CONSTANT(0), _CMP_NEQ_UQ) == 0)
                        break;
                    
                    // z_{n+1} = z_n^2 + 2z_n + c
                    // X = XX - YY - X - X + X0;
                    // Y = XY + XY + Y + Y + Y0;
                    
                    // z_{n+1} = z_n^3 + c
                    // X = XX * X - VECTOR_F_CONSTANT(3) * X * YY + X0;
                    // Y = -VECTOR_F_CONSTANT(3) * XY * Y - YY * Y + Y0;

                    //z_{n+1} = z_n^4 + c
                    X = XX * XX - VECTOR_F_CONSTANT(6) * XX * YY + YY*YY;
                    Y = 4 * XX * X * Y - 4 * X * YY * Y;                
                }
            }

            for (int i = 0; i < param_number; i++)
            {
                if (steps[i])
                    image->setPixel(pixel_x + i, pixel_y, StepToColor(steps[i]));
            }           
        }
    }
}
#endif // AVX512

size_t RunVersion(sf::Image* image, MandelbrotParams* params) {    
    InitTimer();

    StartTimer();
    #ifdef AVX512
        ConstructMandelbrotAVX512UsefulFormat(image, params);
    #else
        #ifdef AVX256
            ConstructMandelbrotSSE(image, params);
        #else 
            ConstructMandelbrotV1(image, params);
        #endif
    #endif
    
    StopTimer();
    
    return GetTimerMicroseconds();
}
