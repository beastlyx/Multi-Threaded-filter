# Borys Banaszkiewicz - Parallel Image Filtering

## Overview
This project implements a multi-threaded image filtering application in C. It leverages the power of multithreading to 
apply various filters to images efficiently. The program can process BMP images quickly by dividing the workload among 
multiple threads, making it significantly faster than single-threaded implementations.

Before and After applying filter to an image:  
![Before](Parallel-Image-Filtering/test1wonderbread.bmp) ![After](Parallel-Image-Filtering/test1_output.bmp)

## Algorithms used

### Box blur
This function applies a blur effect to an image. It uses a 3x3 grid of neighboring pixels to calculate the new color for 
each pixel. For each pixel in the image, it averages the red, green, and blue values of its valid neighbors and assigns 
these averaged values back to the pixel, resulting in a blurred effect.
![BoxBlur](Parallel-Image-Filtering/BoxBlur.png)

### Generating holes
This algorithm is designed to generate random holes that are evenly distributed along the x and y-axis on an image. The 
holes have different sizes (small, medium, large), and their positions are calculated to ensure an even distribution. 
Additionally, a smoothing effect is applied to the edges of the holes for a more natural look.

#### calculate_holes algorithm
This method determines the number of holes and the size of each hole. It calculates the count of small, medium, and 
large holes based on the total number of holes where medium-sized holes are the most common to provide a balanced look, 
and small and large are less common to add diversity. The method then assigns a size to each hole, which will scale with 
different sized images, and stores these values in an array. This array is shuffled to randomize the order of the hole 
sizes.
![CalculateHoles](Parallel-Image-Filtering/CalculateHoles.png)  


#### draw_holes algorithm
This function begins by calculating the smoothing radius for each hole size. This is done to enhance the appearance of 
the holes, making their edges sharper and more defined. It then iterates over a section of the image, determined by the 
thread's workload, checking each pixel to see if it falls within the radius of a hole. If a pixel is within the hole's 
radius, it sets the pixel color to black. Additionally, if the pixel is within the smoothing radius, a gradient effect 
is applied to blend the hole smoothly into the surrounding area.

**Thread Coordination**: Each thread works on a specific portion of the image to avoid overlap and ensure efficient 
processing. The threads calculate their bounds to ensure they only work within their assigned section.
![DrawHoles](Parallel-Image-Filtering/DrawHoles.png)


#### calculate_random_coordinates algorithm
This method first 
calculates the dimensions of a NxM grid that divides the image into evenly sized sections, with each section containing 
one hole. The method then generates random coordinates within each grid section, ensuring that the holes are randomly 
spaced and not clustered together. The coordinates are stored in an array, which is used by the draw_holes method to 
determine the center of each hole.
![RandomCoordinates](Parallel-Image-Filtering/RandomCoordinates.png)
