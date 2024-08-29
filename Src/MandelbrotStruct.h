#ifndef MANDELBROT_STRUCT
#define MANDELBROT_STRUCT

#include <cstddef>

struct Rectangle
{
    float LeftBorder   = 0;
    float BottomBorder = 0;
    float RightBorder  = 0;
    float UpBorder     = 0;
};

struct MandelbrotParams
{
    Rectangle set_border   = {-2.5, -1.25, 1.5, 1.35}; //! borders of set in coordinate plane
    size_t    image_width  = 1920;                //! width in pixel of image in which mandelbrot set need to draw
    size_t    image_height = 1080;                //! height in pixel of image in which mandelbrot set need to draw
    float     radius_2     = 100;                //! square of radius for mandelbrot set calculation
    size_t    iterations   = 256;                //! number of iterations after which point considered stable    
    double    zoom_lvl     = 1;                  //! this parameter uses in moving 
};

const size_t kTimeCalcMandelbrotSet = 1;

#endif // MANDELBROT_STRUCT