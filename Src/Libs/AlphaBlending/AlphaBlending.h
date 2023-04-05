#ifndef SYM_ALPHA_BLENDING
#define SYM_ALPHA_BLENDING

#include "../ParseBmp/ParseBmp.h"

sf::Image AlphaBlendingV0(sf::Image background, sf::Image foreground);
void      AlphaBlendingV1(Image_t* result, const Image_t* background, const Image_t* foreground);
void      AlphaBlendingAVX512(Image_t* result, const Image_t* backgr, const Image_t* foregr);
void      MakeAlphaBlending(const char* background_path, const char* foreground_path);

#endif