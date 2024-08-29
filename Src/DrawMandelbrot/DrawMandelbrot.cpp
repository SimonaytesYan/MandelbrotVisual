#include <math.h>

#include "DrawMandelbrot.h"
#include "../Stopwatch.h"
#include "../CalcMandelbrot/CalcMandelbrot.h"

#define DRAW

//==============================WINDOW CONSTS===================================
const size_t kMaxFpsStrLen   = 20;
const char   kWindowHeader[] = "Mandelbrot set";

//==============================OTHER CONSTS====================================
const float   kMovingSpeed = 0.1;
const float   kZoomSpeed   = 0.05;

//==============================FUNCTIONS PROTOTIPE===============================
static        void      ProcessSetMoving(MandelbrotParams *params);
static        void      UpdateFpsViewer(sf::Text *fps_counter, float fps);
static        double    CalculateFPS(size_t miliseconds);

void DrawMandelbrotSet(MandelbrotParams params)
{
    sf::RenderWindow window(sf::VideoMode(params.image_width, params.image_height), kWindowHeader, sf::Style::Default);

    sf::Image image;
    image.create(params.image_width, params.image_height, sf::Color::Black);

    sf::Texture image_texture;
    sf::Sprite  drawable;
    sf::Text    fps_counter;
    sf::Font    font;

    font.loadFromFile("Font.ttf");    
    fps_counter.setFont(font);
    fps_counter.setFillColor(sf::Color::White);
    fps_counter.setPosition(0, 0);

    while (window.isOpen())
    {
        size_t run_miliseconds = RunVersion(&image, &params);

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
            UpdateFpsViewer(&fps_counter, CalculateFPS(run_miliseconds));

            window.clear();
            image_texture.loadFromImage(image);
            drawable.setTexture(image_texture);

            window.draw(drawable);
            window.draw(fps_counter);
            window.display();
        #else
            printf("FPS = %g\n", ((1/(float)(Get_timer_microseconds)) * 1000000));
        #endif
    }
}

//=============================================OTHER FUNCTIONS=================================

static double CalculateFPS(size_t milliseconds) {
    if (milliseconds == 0)
        return INFINITY;

    return ((1/(float)(milliseconds)) * 1000000. * (double)kTimeCalcMandelbrotSet);
}

static void UpdateFpsViewer(sf::Text *fps_counter, float fps)
{
    char fps_str[kMaxFpsStrLen] = {};
    sprintf(fps_str, "%g", fps);

    fps_counter->setString(fps_str);
}

static void ProcessSetMoving(MandelbrotParams *params)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        params->set_border.LeftBorder  -= kMovingSpeed*params->zoom_lvl;
        params->set_border.RightBorder -= kMovingSpeed*params->zoom_lvl;
    }
        
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        params->set_border.LeftBorder  += kMovingSpeed*params->zoom_lvl;
        params->set_border.RightBorder += kMovingSpeed*params->zoom_lvl;
    }
        
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        params->set_border.UpBorder     -= kMovingSpeed*params->zoom_lvl;
        params->set_border.BottomBorder -= kMovingSpeed*params->zoom_lvl;
    }
        
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        params->set_border.UpBorder     += kMovingSpeed*params->zoom_lvl;
        params->set_border.BottomBorder += kMovingSpeed*params->zoom_lvl;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
    {
        const double height_width_coefficient = (params->set_border.UpBorder - params->set_border.BottomBorder) / 
                                                (params->set_border.RightBorder - params->set_border.LeftBorder);

        params->set_border.UpBorder     -= kZoomSpeed * params->zoom_lvl * height_width_coefficient;
        params->set_border.BottomBorder += kZoomSpeed * params->zoom_lvl * height_width_coefficient;
        params->set_border.RightBorder  -= kZoomSpeed * params->zoom_lvl;
        params->set_border.LeftBorder   += kZoomSpeed * params->zoom_lvl;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
    {
        const double height_width_coefficient = (params->set_border.UpBorder - params->set_border.BottomBorder) / 
                                                (params->set_border.RightBorder - params->set_border.LeftBorder);

        params->set_border.UpBorder     += kZoomSpeed * params->zoom_lvl * height_width_coefficient;
        params->set_border.BottomBorder -= kZoomSpeed * params->zoom_lvl * height_width_coefficient;
        params->set_border.RightBorder  += kZoomSpeed * params->zoom_lvl;
        params->set_border.LeftBorder   -= kZoomSpeed * params->zoom_lvl;
    }
}