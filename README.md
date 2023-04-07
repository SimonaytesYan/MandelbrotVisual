## Testing machine
 |||
-------------------|-
**Compiler**           | g++ 11.3.0 
**OS**                 | Ubuntu 11.3.0 
**CPU**                | 11th Gen Intel(R) Core(TM) i7-1165G7 @ 2.80GHz

# Alpha blending #

Stage                                                                                               | FPS   
----------------------------------------------------------------------------------------------------|:-------------------
 Naive implementation + debug flags                                                                 | **21    - 22**
 AVX512 + debug flags + put processing part of background in one-dimensional array                  | **150   - 155**
 Naive implementation + -O3                                                                         | **312   - 320**
 Naive implementation + -O3 and -march=native flags                                                 | **312   - 320**
 AVX512 + -O3 + put processing part of background in one-dimensional array + aligned load and store | **550   - 557**
 AVX512 + -O3 + use not aligned load and store instructions                                         | **1200  - 1210**
 AVX512 + -O3 + use not aligned load and store instructions(reorginise code)                        | **1190  - 1210**

# Mandelbrot set visualisation #

![Mandelbrot set](/Output/MandelbrotImage.png)

## Hot keys ##
**WASD**   - moving\
**Q**      - zoom in\
**E**      - zoom out

### Notice ##
    All *.0 versions compile without optimisation flags

## Results ##
Version | Stage                                                | FPS   
--------|------------------------------------------------------|:-----
1.0     | Without any optimizations + debug flags              | **5-6**
1.1     | Version 1.0 compile with -O3 and -msse4 flags        | **10-11**
2.0     | In each internal cycle now process 4 points          | **0.13-0.14**
2.1     | Version 2.0 compile with -O3 and -msse4 flags        | **5-6**
3.0     | Each for(i = 0, ..., 3) put in functions             | **0.07-0.08**
3.1     | Version 3.0 compile with -O3 and -msse4 flags        | **4-5**
4       | Use sse intrinsic function with -O3 and -msse4 flags | **37-40**
5       | Use avx512 intrinsic functions                       | **100-110**