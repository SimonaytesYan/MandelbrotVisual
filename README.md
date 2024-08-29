# Mandelbrot set visualization #

This program created to test SIMD instructions. It calculate Mandelbrot and visualize Mandelbrot set using SFML. 
Every frame set have be recalculate and print FPS 

![Mandelbrot set](/Output/MandelbrotImage.png)

## How to build project ##
To build project with using avx512 you should run
```
make avx512
```
To build project with using sse 4.2 you should run
```
make sse
```
To build project without using simd you should run
```
make without_simd
```
To build project with debug flags you should run
```
make debug
```

To run project run
```
make run
```

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
All results calculate without `DRAW` define, so mandelbrot set didn`t vi 
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