#include <SFML/Graphics.hpp>
#include "AlphaBlending.h"
#include "../Stopwatch.h"

#define DRAW

const size_t kMaxFpsStrLen   = 20;

const char   kWindowHeader[]     = "Alpha blending";
const size_t kTimeCalcAlphaBlend = 1;

static void UpdateFpsViewer(sf::Text *fps_counter, float fps);

void MakeAlphaBlending(char* background_path, char* foreground_path)
{
    sf::Image background;
    sf::Image foreground;

    background.loadFromFile(background_path);
    foreground.loadFromFile(foreground_path);

    sf::Vector2u image_size = background.getSize();

    sf::RenderWindow window(sf::VideoMode(image_size.x, image_size.y), kWindowHeader, sf::Style::Default);

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
        //ConstructMandelbrotV1(&image, &params);
        sf::Image result = AlphaBlending(background, foreground);
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
                    default:
                        break;
                }
            }
            UpdateFpsViewer(&fps_counter, ((1/(float)(GetTimerMicroseconds())) * 1000000. * (double)kTimeCalcAlphaBlend));

            window.clear();

            image_texture.loadFromImage(result);
            sprite.setTexture(image_texture);

            window.draw(sprite);
            window.draw(fps_counter);
            window.display();
        #else
            printf("FPS = %g\n", ((1/(float)(GetTimerMicroseconds)) * 1000000));
        #endif
    }
}

sf::Image AlphaBlending(sf::Image background, sf::Image foreground)
{
    sf::Vector2u fg_size = foreground.getSize();

    for (size_t y = 0; y < fg_size.y; y++)
    {
        for (size_t x = 0; x < fg_size.x; x++)
        {
            sf::Color fg = foreground.getPixel(x, y);
            sf::Color bg = background.getPixel(x, y);

            sf::Color res_pixel((fg.r * fg.a + bg.r * (255 - fg.a)) >> 8,
                                (fg.r * fg.a + bg.r * (255 - fg.a)) >> 8,
                                (fg.r * fg.a + bg.r * (255 - fg.a)) >> 8);

            background.setPixel(x, y, res_pixel);
        }
    }

    return background;
}

static void UpdateFpsViewer(sf::Text *fps_counter, float fps)
{
    char fps_str[kMaxFpsStrLen] = {};
    sprintf(fps_str, "%g", fps);

    fps_counter->setString(fps_str);
}