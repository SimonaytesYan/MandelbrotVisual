# Mandelbrot set visualisation #

![Mandelbrot set](/Output/MandelbrotImage.png)


## What was tested on
 |||
-------------------|-
**Compiler**           | g++ 11.3.0 
**OS**                 | Ubuntu 11.3.0 
**CPU**                | 11th Gen Intel(R) Core(TM) i7-1165G7 @ 2.80GHz

### Notice ##
    All *.0 versions compile without optimisation flags

## Results ##
Version | Stage                                          | FPS   
--------|------------------------------------------------|:-----:
1.0     | Without any optimizations + debug flags        | 5-6
1.1     | Version 1.0 compile with -O3 and -msse4 flags  | 10-11
2.0     | In each internal cycle now process 4 points    | 0.13-0.14
2.1     | Version 2.0 compile with -O3 and -msse4 flags  | 5-6
3.0     | Each for(i = 0, ..., 3) put in functions       | 0.07-0.08
3.1     | Version 3.0 compile with -O3 and -msse4 flags  | 4-5
4.0     | Use sse intrinsic function                     | 8-9 
4.1     | Version 4.0 compile with -O3 and -msse4 flags  | 37-40