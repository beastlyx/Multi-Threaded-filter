# Borys Banaszkiewicz - Parallel Image Filtering

## Overview
This project implements a multi-threaded image filtering application in C. It leverages the power of multithreading to apply various filters to images efficiently. The program can process BMP images
quickly by dividing the workload among multiple threads, making it significantly faster than single-threaded implementations.

Supported Filters:
 - grayscale
 - swiss cheese
 - blur

Before and After applying filter to an image:
![Before](Parallel-Image-Filtering/test1wonderbread.bmp) ![After](Parallel-Image-Filtering/test1_output.bmp)

## Algorithms used

### Box blur
![BoxBlur](Parallel-Image-Filtering/BoxBlur.png)

### Yellow filter
![YellowFilter](Parallel-Image-Filtering/YellowFilter.png)

### Generating holes
![CalculateHoles](Parallel-Image-Filtering/CalculateHoles.png)
![DrawHoles](Parallel-Image-Filtering/DrawHoles.png)
![RandomCoordinates](Parallel-Image-Filtering/RandomCoordinates.png)
