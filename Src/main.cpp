#include <math.h>
#include <SFML/Graphics.hpp>

#include "Libs/DrawMandelbrot/DrawMandelbrot.h"
#include "Libs/AlphaBlending/AlphaBlending.h"
#include "Libs/AlphaBlendingAVX/AlphaBlendingAVX.h"
#include "Libs/Stopwatch.h"
#include "Libs/AlignedCalloc/AlignedCalloc.h"

#define DRAW

const size_t kMaxFpsStrLen = 20;

const char   kWindowHeader[]     = "Alpha blending";
const size_t kNumberMeas         = 100;        
const size_t kTimeCalcAlphaBlend = 100;

void   UpdateFpsViewer(sf::Text *fps_counter, float fps);
void   MakeAlphaBlending(const char* background_path, const  char* foreground_path);
void   DrawImage(sf::RenderWindow* window, Image_t* result, sf::Sprite* sprite, sf::Texture* image_texture, sf::Text* fps_counter, double microsec);
double CulcADnPrintfStdDeviation(const double fps[], const size_t number_meas);

int main()
{
    MakeAlphaBlending("Images/Forest.bmp", "Images/BigCat.bmp");
}

void MakeAlphaBlending(const char* background_path, const  char* foreground_path)
{
    Image_t background = {};
    Image_t foreground = {};

    GetImageFromBMP(&background, background_path, 64);
    GetImageFromBMP(&foreground, foreground_path, 64);

    Image_t result = {};

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

    size_t numbr_meas = 0;
    double fps[kNumberMeas] = {0};

    while (window.isOpen())
    {    
        InitTimer();
        StartTimer();
        //AlphaBlendingAVX512(&result, &background, &foreground, kTimeCalcAlphaBlend);
        //AlphaBlendingV1(&result, &background, &foreground, kTimeCalcAlphaBlend);
        AlphaBlendingAVX512_V2(&result, &background, &foreground, kTimeCalcAlphaBlend);
        StopTimer();

        double last_fps  = 1/(double)GetTimerMicroseconds()* 1000. * (double)kTimeCalcAlphaBlend;
        fps[numbr_meas]  = last_fps;
        numbr_meas++;
        if (numbr_meas == kNumberMeas)
            CulcADnPrintfStdDeviation(fps, numbr_meas);

        #ifdef DRAW
            DrawImage(&window, &result, &sprite, &image_texture, &fps_counter, GetTimerMicroseconds());
        #else
            printf("FPS = %lf\n", ((1/(float)(GetTimerMicroseconds())) * 1000000));
        #endif
    }

    AlignedFree((void**)&result.real_array_ptr);
}

void   DrawImage(sf::RenderWindow* window, Image_t* result, sf::Sprite* sprite, sf::Texture* image_texture, sf::Text* fps_counter, double microsec)
{
    sf::Event event;
    while (window->pollEvent(event))
    {
        if (sf::Event::Closed)
        {
            window->close();
            break;
        }
    }
    if (microsec != 0)
        UpdateFpsViewer(fps_counter, (1/microsec) * 1000. * (double)kTimeCalcAlphaBlend);

    sf::Image res_image;
    res_image.create(result->info.w, result->info.h, sf::Color::White);

    for (int y = 0; y < result->info.h; y++)
    {
        for (int x = 0; x < result->info.w; x++)
        {
            res_image.setPixel(x, result->info.h - y - 1, sf::Color(result->pixels[y * result->info.w + x].r, 
                                                                    result->pixels[y * result->info.w + x].g, 
                                                                    result->pixels[y * result->info.w + x].b));
        }
    }

    window->clear();
    
    image_texture->loadFromImage(res_image);
    sprite->setTexture(*image_texture);
    sprite->setScale(sf::Vector2f(window->getSize().x / (float)result->info.w, 
                                  window->getSize().y / (float)result->info.h));

    window->draw(*sprite);
    window->draw(*fps_counter);
    window->display();
}

double CulcADnPrintfStdDeviation(const double fps[], const size_t number_meas)
{
    double sum_fps = 0;
    for (int i = 0; i < kNumberMeas; i++)
        sum_fps += fps[i];

    double av_fps        = sum_fps/number_meas;
    double std_deviation = 0;

    for (int i = 0; i < kNumberMeas; i++)
        std_deviation += (fps[i] - av_fps) * (fps[i] - av_fps);
    std_deviation /= (double)(kNumberMeas - 1);

    std_deviation = sqrt(std_deviation);
    printf("av_fps        = %lg\n", av_fps);
    printf("std_deviation = %lg\n", std_deviation);

    return std_deviation;
}

void UpdateFpsViewer(sf::Text *fps_counter, float fps)
{
    char fps_str[kMaxFpsStrLen] = {};
    sprintf(fps_str, "%lf", fps);

    fps_counter->setString(fps_str);
}