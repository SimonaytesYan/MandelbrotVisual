#include <stdlib.h>
#include <SFML/Graphics.hpp>
#include <sys/time.h>

#define Init_timer              \
    struct timeval stop, start;
#define Start_timer             \
    gettimeofday(&start, NULL);

#define Stop_timer              \
    gettimeofday(&stop, NULL);

#define Get_timer_microseconds       \
    (((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec))

//===============================MANDELBROT CONSTS==============================
const float first_x = -2;
const float first_y = -1.5;
const float last_x  = 1;
const float last_y  = 1.5;

const size_t ITERATION_NUMBER = 800;
const float segment_length_x = last_x - first_x;
const float segment_length_y = last_y - first_y;

const float delta_x          = segment_length_x/ITERATION_NUMBER;
const float delta_y          = segment_length_y/ITERATION_NUMBER;

const size_t RADIUS_2        = 100;
const size_t MAX_ITERATIONS  = 256;

//==============================WINDOW CONSTS===================================
const size_t WINDOW_HEIGHT    = 800;
const size_t WINDOW_WEIGHT    = 800;
const size_t MAX_FPS_STR_LEN  = 20;

const char* HEADER = "Mandelbrot set";

void UpdateFpsViewer(sf::Text *fps_viewer, size_t fps)
{
    char fps_str[MAX_FPS_STR_LEN] = {};
    sprintf(fps_str, "%zu", fps);

    fps_viewer->setString(fps_str);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_WEIGHT, WINDOW_HEIGHT), HEADER, sf::Style::Default);

    sf::Image image = sf::Image();
    image.create(WINDOW_WEIGHT, WINDOW_HEIGHT, sf::Color::Black);

    sf::Texture image_texture = sf::Texture();
    sf::Sprite  drawble       = sf::Sprite();
    sf::Text    fps_viewer    = sf::Text();
    sf::Font    font          = sf::Font();

    font.loadFromFile("Font.ttf");    
    fps_viewer.setFont(font);
    fps_viewer.setFillColor(sf::Color::White);
    fps_viewer.setPosition(0, 0);

    Init_timer;

    while (window.isOpen())
    {
        Start_timer;

        float x0 = first_x;
        for (size_t pixel_x = 0; pixel_x < ITERATION_NUMBER; pixel_x++, x0 += delta_x)
        {
            float y0 = first_y;
            for(size_t pixel_y = 0; pixel_y < ITERATION_NUMBER; pixel_y++, y0 += delta_y)
            {
                float X = x0;
                float Y = y0;

                bool draw_pixel = true;

                int i = 0;
                for (i = 0; i < MAX_ITERATIONS; i++)
                {
                    if (RADIUS_2 < X*X + Y*Y)
                    {
                        draw_pixel = false;
                        break;
                    }

                    float new_x = X*X - Y*Y + x0;
                    float new_y = 2*X*Y + y0;
                    X = new_x;
                    Y = new_y;
                }

                int k = 0;

                if (!draw_pixel)
                    image.setPixel(pixel_x, pixel_y, sf::Color((i*10)%256, i, i));
                
            }
        }

        Stop_timer;

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        UpdateFpsViewer(&fps_viewer, (size_t)((1/(float)(Get_timer_microseconds)) * 1000000));

        window.clear();
        image_texture.loadFromImage(image);
        drawble.setTexture(image_texture);

        window.draw(drawble);
        window.draw(fps_viewer);
        window.display();
    }
}