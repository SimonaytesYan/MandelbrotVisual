#ifndef SYM_DRAW_MANDELBROT
#define SYM_DRAW_MANDELBROT

#include <SFML/Graphics.hpp>

struct Rectangle
{
    float LeftBoder   = 0;
    float BottomBoder = 0;
    float RightBoder  = 0;
    float UpBoder     = 0;
};

struct MandelbrotParams
{
    Rectangle set_border   = {-2, -1.5, 1, 1.5}; //!borders of set in coordinate plane
    size_t    image_width  = 800;                //!width in pixel of image in which mandelbrot set need to draw
    size_t    image_height = 800;                //!height in pixel of image in which mandelbrot set need to draw
    size_t    radius_2     = 100;                //!square of radius for mandelbrot set calcultion
    size_t    iterations   = 256;                //!number of iterations after which point considered stable    
};

void DrawMandelbrotSet(MandelbrotParams params = {});

void ConstructMandelbrotAVX512(sf::Image* image, MandelbrotParams* params);
void ConstructMandelbrotSSE   (sf::Image* image, MandelbrotParams* params);
void ConstructMandelbrotV1    (sf::Image* image, MandelbrotParams* params);
void ConstructMandelbrotV2    (sf::Image* image, MandelbrotParams* params);
void ConstructMandelbrotV3    (sf::Image* image, MandelbrotParams* params);

#endif