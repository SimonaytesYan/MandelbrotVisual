#ifndef SYM_DRAW_MANDELBROT
#define SYM_DRAW_MANDELBROT

#include <SFML/Graphics.hpp>

void DrawMandelbrotSet();

void ConstructMandelbrotSSE(sf::Image* image);
void ConstructMandelbrotAVX512(sf::Image* image);
void ConstructMandelbrotV1(sf::Image* image);
void ConstructMandelbrotV2(sf::Image* image);
void ConstructMandelbrotV3(sf::Image* image);

#endif