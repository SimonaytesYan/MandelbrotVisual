#ifndef SYM_ALPHA_BLENDING_AVX_256
#define SYM_ALPHA_BLENDING_AVX_256

#include "../ParseBmp/ParseBmp.h"

void AlphaBlendingAVX256(Image_t* result, const Image_t* backgr, const Image_t* foregr, const size_t kTimeCalcAlphaBlend);

#endif