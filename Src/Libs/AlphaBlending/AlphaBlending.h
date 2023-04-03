#ifndef SYM_ALPHA_BLENDING
#define SYM_ALPHA_BLENDING

struct Pixel_t
{
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    unsigned char a = 0;
};

struct Image_t
{
    Pixel_t* pixels = nullptr;
    size_t   width  = 0;
    size_t   height = 0;
};


sf::Image AlphaBlendingV0(sf::Image background, sf::Image foreground);
void      AlphaBlendingV1(Image_t* result, const Image_t* background, const Image_t* foreground);
void      MakeAlphaBlending(char* background_path, char* foreground_path);

#endif