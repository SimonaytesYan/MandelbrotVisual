# Alpha blending #

Stage                                                                              | FPS(10^3)   
-----------------------------------------------------------------------------------|:-------------------
 Naive implementation + debug flags                                                | **24    - 24,5**
 AVX512 + debug flags + put processing part of background in one-dimensional array | **154   - 156,5**
 Naive implementation + -O3 flags                                                  | **472,5 - 474,5**
 AVX512 + -O3 + put processing part of background in one-dimensional array         | **504,5 - 501,5**
 AVX512 + -O3 + use not aligned load and store instructions                        | **1172  - 1190**

# Mandelbrot set visualisation #

![Mandelbrot set](/Output/MandelbrotImage.png)

## Hot keys ##
**WASD**   - moving\
**Q**      - zoom in\
**E**      - zoom out

## What was tested on
 |||
-------------------|-
**Compiler**           | g++ 11.3.0 
**OS**                 | Ubuntu 11.3.0 
**CPU**                | 11th Gen Intel(R) Core(TM) i7-1165G7 @ 2.80GHz

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