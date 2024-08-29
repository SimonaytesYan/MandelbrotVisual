#ifndef SYM_DRAW_MANDELBROT
#define SYM_DRAW_MANDELBROT

#include <SFML/Graphics.hpp>
#include "../MandelbrotStruct.h"

void DrawMandelbrotSet(MandelbrotParams params = {});

void ConstructMandelbrotAVX512UsefulFormat(sf::Image* image, MandelbrotParams* params);
void ConstructMandelbrotAVX512            (sf::Image* image, MandelbrotParams* params);
void ConstructMandelbrotSSE               (sf::Image* image, MandelbrotParams* params);
void ConstructMandelbrotV1                (sf::Image* image, MandelbrotParams* params);
void ConstructMandelbrotV2                (sf::Image* image, MandelbrotParams* params);
void ConstructMandelbrotV3                (sf::Image* image, MandelbrotParams* params);

#endif