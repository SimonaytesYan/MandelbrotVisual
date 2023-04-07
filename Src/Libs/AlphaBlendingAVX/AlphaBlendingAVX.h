#ifndef SYM_ALPHA_BLENDING_AVX
#define SYM_ALPHA_BLENDING_AVX

#include "../ParseBmp/ParseBmp.h"

void AlphaBlendingAVX512(Image_t* result, const Image_t* backgr, const Image_t* foregr, const size_t kTimeCalcAlphaBlend);
void AlphaBlendingAVX512_V1(Image_t* result, const Image_t* backgr, const Image_t* foregr, const size_t kTimeCalcAlphaBlend);
void AlphaBlendingAVX512_V2(Image_t* result, const Image_t* backgr, const Image_t* foregr, const size_t kTimeCalcAlphaBlend);

#endif