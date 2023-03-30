#include <stdlib.h>
#include <SFML/Graphics.hpp>
#include <emmintrin.h>

#include "Libs/Stopwatch.h"

#define DRAW

//===============================MANDELBROT CONSTS==============================
const float  kLeftAreaBoder    = -2;
const float  kBottomAreaBoder = -1.5;
const float  kRightAreaBoder   = 1;
const float  kUpAreaBoder      = 1.5;

const size_t kIterationNumber = 800;

const float  kDrawAreaX     = kRightAreaBoder - kLeftAreaBoder;
const float  kDrawAreaY     = kUpAreaBoder - kBottomAreaBoder;
const float  kDeltaX        = kDrawAreaX/kIterationNumber;
const float  kDeltaY        = kDrawAreaY/kIterationNumber;

const size_t kMaxIterations = 256;
const size_t kRadius2       = 100;
const __m128 kRadius2m128   = _mm_set_ps1(kRadius2);

//==============================WINDOW CONSTS===================================
const size_t kWindowHeight   = 800;
const size_t kWindowWidth    = 800;
const size_t kMaxFpsStrLen   = 20;
const char   kWindowHeader[] = "Mandelbrot set";

//==============================OTHER CONSTS====================================
const float _3210[4] = {0, 1, 2, 3};

//==============================FUNCTIONS PROTOTIPE===============================
void UpdateFpsViewer(sf::Text *fps_counter, float fps);
void DrawMandelbrotSSE(sf::Image* image);
void DrawMandelbrotV1(sf::Image* image);
void DrawMandelbrotV2(sf::Image* image);
void DrawMandelbrotV3(sf::Image* image);

//==============================FOR WRAPERS======================================
inline void mm_set_ps1(float* A, const float  elem) {for(int i = 0; i < 4; i++) A[i] = elem;}
inline void mm_mul_ps (float* A, const float* B)    {for(int i = 0; i < 4; i++) A[i] = A[i] * B[i];}
inline void mm_add_ps (float* A, const float* B)    {for(int i = 0; i < 4; i++) A[i] = A[i] + B[i];}
inline void mm_sub_ps (float* A, const float* B)    {for(int i = 0; i < 4; i++) A[i] = A[i] - B[i];}
inline void mm_mov_ps (float* A, const float* B)    {for(int i = 0; i < 4; i++) A[i] = B[i];}

int main()
{
    sf::RenderWindow window(sf::VideoMode(kWindowWidth, kWindowHeight), kWindowHeader, sf::Style::Default);

    sf::Image image;
    image.create(kWindowWidth, kWindowHeight, sf::Color::Black);

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
        DrawMandelbrotSSE(&image);
        StopTimer();

        #ifdef DRAW
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();
            }
            UpdateFpsViewer(&fps_counter, ((1/(float)(GetTimerMicroseconds())) * 1000000));

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
void DrawMandelbrotV1(sf::Image* image)
{
    float x0 = kLeftAreaBoder;
    for (size_t pixel_x = 0; pixel_x < kIterationNumber; pixel_x++, x0 += kDeltaX)
    {
        float y0 = kBottomAreaBoder;
        for(size_t pixel_y = 0; pixel_y < kIterationNumber; pixel_y++, y0 += kDeltaY)
        {
            float X = x0;
            float Y = y0;

            bool draw_pixel = true;

            int i = 0;
            for (i = 0; i < kMaxIterations; i++)
            {
                if (kRadius2 < X*X + Y*Y)
                {
                    draw_pixel = false;
                    break;
                }

                float new_x = X*X - Y*Y + x0;
                float new_y = 2*X*Y + y0;
                X = new_x;
                Y = new_y;
            }

            if (!draw_pixel)
                image->setPixel(pixel_x, pixel_y, sf::Color((i*10)%256, i, i));
            
        }
    }
}

//======================VERSION 2=====================
void DrawMandelbrotV2(sf::Image* image)
{
    float y0 = kBottomAreaBoder;
    for(size_t pixel_y = 0; pixel_y < kIterationNumber; pixel_y++, y0 += kDeltaY)
    {
        float x0 = kLeftAreaBoder;
        for (size_t pixel_x = 0; pixel_x < kIterationNumber; pixel_x+=4, x0 += 4*kDeltaX)
        {
            float X0[4] = {x0, x0 + kDeltaX, x0 + kDeltaX*2, x0 + kDeltaX*3};
            float Y0[4] = {y0, y0, y0, y0};
            float X[4]  = {}; for(int i = 0; i < 4; i++) X[i] = X0[i];
            float Y[4]  = {y0, y0, y0, y0};
            
            int draw_pixel = 0b1111;

            int steps[4] = {};
            for (int n = 0; n < kMaxIterations; n++)
            {
                float XX[4] = {};   for (int i = 0; i < 4; i++) XX[i] = X[i]*X[i];
                float YY[4] = {};   for (int i = 0; i < 4; i++) YY[i] = Y[i]*Y[i];
                float XY[4] = {};   for (int i = 0; i < 4; i++) XY[i] = X[i]*Y[i];

                for(int i = 0; i < 4; i++)
                {
                    if ((draw_pixel & (1 << i)) && kRadius2 < XX[i] + YY[i])
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

            for (int i = 0; i < 4; i++)
            {
                if (draw_pixel & (1 << i))
                    image->setPixel(pixel_x + i, pixel_y, sf::Color((steps[i]*10)%256, steps[i], steps[i]));
            }
            
        }
    }
}

//======================VERSION 3=====================
void DrawMandelbrotV3(sf::Image* image)
{

    float y0 = kBottomAreaBoder;
    for(size_t pixel_y = 0; pixel_y < kIterationNumber; pixel_y++, y0 += kDeltaY)
    {
        float x0 = kLeftAreaBoder;
        for (size_t pixel_x = 0; pixel_x < kIterationNumber; pixel_x+=4, x0 += 4*kDeltaX)
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
                
            int draw_pixel = 0b1111;

            int N[4] = {};
            int n = 0;
            for (n = 0; n < kMaxIterations; n++)
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
                    if ((draw_pixel & (1 << i)) && kRadius2 < XX[i] + YY[i])
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

            for (int i = 0; i < 4; i++)
            {
                if (draw_pixel & (1 << i))
                    image->setPixel(pixel_x + i, pixel_y, sf::Color((N[i]*10)%256, N[i], N[i]));
            }
                
        }
    }
}

//======================VERSION 4=====================
void DrawMandelbrotSSE(sf::Image* image)
{
    float y0 = kBottomAreaBoder;
    for(size_t pixel_y = 0; pixel_y < kIterationNumber; pixel_y++, y0 += kDeltaY)
    {
        float x0 = kLeftAreaBoder;
        for (size_t pixel_x = 0; pixel_x < kIterationNumber; pixel_x += 4, x0 += 4*kDeltaX)
        {
            __m128 X0 = _mm_set_ps(x0 + kDeltaX*3, x0 + kDeltaX*2, x0 + kDeltaX, x0);
            __m128 Y0 = _mm_set_ps1(y0);
            __m128 X  = X0;
            __m128 Y  = Y0;
                
            __m128i steps = _mm_setzero_si128();   

            for (int n = 0; n < kMaxIterations; n++)
            {
                __m128 XX = _mm_mul_ps(X, X);
                __m128 YY = _mm_mul_ps(Y, Y);
                __m128 XY = _mm_mul_ps(X, Y);

                __m128 radius_2 = _mm_add_ps(XX, YY);

                __m128 cmp = _mm_cmpge_ps(kRadius2m128, radius_2);
                if (!_mm_movemask_ps(cmp))
                    break;
                    
                steps = _mm_sub_epi32(steps, _mm_castps_si128(cmp));
                    
                X = _mm_add_ps(_mm_sub_ps(XX, YY), X0);
                Y = _mm_add_ps(_mm_add_ps(XY, XY), Y0);
            }

            int* step_int = (int*)&steps;
            for (int i = 0; i < 4; i++)
            {
                if (step_int[i])
                    image->setPixel(pixel_x + i, pixel_y, sf::Color((step_int[i]*10)%256, step_int[i], step_int[i]));
            }           
        }
    }
}

void UpdateFpsViewer(sf::Text *fps_counter, float fps)
{
    char fps_str[kMaxFpsStrLen] = {};
    sprintf(fps_str, "%g", fps);

    fps_counter->setString(fps_str);
}