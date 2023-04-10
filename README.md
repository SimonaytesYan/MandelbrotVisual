# Testing machine #
 |||
-------------------|-
**Compiler**           | g++ 11.3.0 
**OS**                 | Ubuntu 11.3.0 
**CPU**                | 11th Gen Intel(R) Core(TM) i7-1165G7 @ 2.80GHz

# Mandelbrot set visualisation #

![Mandelbrot set](/Output/MandelbrotImage.png)

## Results ##
Version | Stage                                                | FPS            |
--------|------------------------------------------------------|:--------------:|
1.0     | Without any optimizations + debug flags              | **5-6**        |
1.1     | Version 1.0 compile with -O3 and -msse4 flags        | **10-11**      |
2.0     | In each internal cycle now process 4 points          | **0.13-0.14**  |
2.1     | Version 2.0 compile with -O3 and -msse4 flags        | **5-6**        |
3.0     | Each for(i = 0, ..., 3) put in functions             | **0.07-0.08**  |
3.1     | Version 3.0 compile with -O3 and -msse4 flags        | **4-5**        |
4       | Use sse intrinsic function with -O3 and -msse4 flags | **37-40**      |
5       | Use avx512 intrinsic functions                       | **100-110**    |

# Alpha blending #

Stage                                                                                               | FPS        | Coefficient | Std deviation 
----------------------------------------------------------------------------------------------------|:-----------|:------------|:------------
 Naive implementation + debug flags                                                                 | **21.3**   |  **5.3%**   |**0.5**
 AVX512 + debug flags + put processing part of background in one-dimensional array  + aligned load and  store                                                                                              | **157**    | **38.8%**   |**0.5**
 AVX512 + debug flags + use not aligned load and store instructions                                 | **284**    | **70.1%**   |**1**
 Naive implementation + -O2                                                                         | **405**    | **100%**    |**0.6**
 Naive implementation + -O2 and -march=native flags                                                 | **406**    | **100.2%**  |**1.3**
 AVX512 + -O2 + put processing part of background in one-dimensional array + aligned load and store | **556**    | **137.3%**  |**1.7**
 AVX512 + -O2 + use not aligned load and store instructions                                         | **1167**   | **288.1%**  |**4.3**
 AVX512 + -O2 + use not aligned load and store instructions(reorginise code)                        | **1202**   | **296.8%**  |**5.8**
