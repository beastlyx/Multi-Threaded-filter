/**
* takes an image and based on user input applies different filters by using a multi-threading technique
*
* Completion time: 50 hours
*
* @author Borys Banaszkiewicz, (anyone else, e.g., Acuna, whose code you used)
* @version 1.0
*/

////////////////////////////////////////////////////////////////////////////////
//INCLUDES
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>

//UNCOMMENT BELOW LINE IF USING SER334 LIBRARY/OBJECT FOR BMP SUPPORT
#include "BmpProcessor.h"

////////////////////////////////////////////////////////////////////////////////
//MACRO DEFINITIONS
//problem assumptions
#define BMP_HEADER_SIZE 14
#define BMP_DIB_HEADER_SIZE 40
#define MAXIMUM_IMAGE_SIZE 4096
#define THREAD_COUNT 11

////////////////////////////////////////////////////////////////////////////////
//DATA STRUCTURES
struct thread_info {
    int start;
    int end;
    struct Pixel** data;
};

struct filter_args {
    struct Pixel** pArr;
    int width;
    int height;
    int start_x;
    int end_x;
    int** coordinates;
    int* radii;
    int holes_total;
    bool blur;
    bool cheese;
};

////////////////////////////////////////////////////////////////////////////////
//MAIN PROGRAM CODE
void box_blur_filter(struct filter_args* args) {
    int offset[][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}, {-1, 0}, {0, -1}, {0, 1}, {1, 0}, {0, 0}};

    for (int h = 0; h < args->height; h++) {
        for (int w = 0; w < args->width; w++) {
            int r = 0, g = 0, b = 0, count = 0;

            for (int i = 0; i < 9; i++) {
                int h_offset = h + offset[i][0];
                int w_offset = w + offset[i][1];

                if (h_offset < 0 || h_offset >= args->height || w_offset < 0 || w_offset >= args->width) continue;

                r += args->pArr[h_offset][w_offset].red;
                g += args->pArr[h_offset][w_offset].green;
                b += args->pArr[h_offset][w_offset].blue;

                count++;
            }

            args->pArr[h][w].red = (unsigned char)(r/count);
            args->pArr[h][w].green = (unsigned char)(g/count);
            args->pArr[h][w].blue = (unsigned char)(b/count);
        }
    }
}

void yellow_filter(struct filter_args* args) {
    for (int h = 0; h < args->height; h++) {
        for (int w = 0; w < args->width; w++) {
            args->pArr[h][w].blue = 0;
        }
    }
}

void draw_holes(struct filter_args* args) {
    int hole_small = pow(args->holes_total * 0.65, 2); // 1.31 smooth
    int hole_medium = pow(args->holes_total, 2); //1.2 smooth
    int hole_large = pow(args->holes_total * 1.35, 2); //1.13 smooth

    for (int i = 0; i < args->holes_total; i++) {
        double radius = sqrt(args->radii[i]);
        int smoothing_radius = args->radii[i] == hole_small ? args->radii[i] * 1.31 : args->radii[i] == hole_medium ? args->radii[i] * 1.2 : args->radii[i] * 1.13;
        int x_center = args->coordinates[i][0];
        int y_center = args->coordinates[i][1];
        int x_left = x_center - ceil(radius);
        int x_right = x_center + ceil(radius);

        if (x_left <= args->end_x && x_right >= args->start_x) {
            for (int h = 0; h < args->height; h++) {
                for (int w = 0; w < args->width; w++) {
                    int distance = pow((w + args->start_x) - x_center, 2) + pow(h - y_center, 2);
                    if (distance <= args->radii[i]) {
                        args->pArr[h][w].red = (unsigned char)0;
                        args->pArr[h][w].green = (unsigned char)0;
                        args->pArr[h][w].blue = (unsigned char)0;
                    }
                    if (distance <= smoothing_radius) {
                        double smooth = (double) (distance - args->radii[i]) / (smoothing_radius - args->radii[i]);
                        args->pArr[h][w].red = (unsigned char) (args->pArr[h][w].red * smooth);
                        args->pArr[h][w].green = (unsigned char) (args->pArr[h][w].green * smooth);
                        args->pArr[h][w].blue = (unsigned char) (args->pArr[h][w].blue * smooth);
                    }
                }
            }
        }
    }
}

int* calculate_holes(struct Pixel** pArr, int height, int width, int holes_total) {
    srand(time(NULL));

    //distribute holes into count of small, medium and large holes (medium being most common)
    int holes_small_count = holes_total % 4 == 0 ? holes_total * 0.25 : holes_total * 0.3;
    int holes_medium_count = holes_total * 0.5;
    int holes_large_count = holes_total * 0.25;

    // calculates the radius^2 of each hole size
    int radius_squared_small = pow(holes_total * 0.65, 2);
    int radius_squared_medium = pow(holes_total, 2);
    int radius_squared_large = pow(holes_total * 1.35, 2);

    // initialize array to store each hole
    int *hole_random = (int*)malloc(sizeof(int)*holes_total);

    for (int i = 0; i < holes_total; i++) {
        if (i < holes_small_count) {
            hole_random[i] = radius_squared_small;
        } else if (i < holes_small_count + holes_medium_count) {
            hole_random[i] = radius_squared_medium;
        } else {
            hole_random[i] = radius_squared_large;
        }
    }

    // shuffles the hole_random array to randomize the order of the hole sizes
    for (int i = 0; i < holes_total; i++) {
        int swap = i + (rand() % (holes_total - i));
        int temp = hole_random[i];
        hole_random[i] = hole_random[swap];
        hole_random[swap] = temp;
    }

    return hole_random;
}

int** calculate_random_coordinates(int height, int width, int holes_total) {
    srand(time(NULL));
    // calculates the nxm (gridHeight x gridWidth) grids that will be used to uniformly distribute the holes
    int gridsVertical = (int) round(sqrt((double) holes_total * width / height));
    int gridsHorizontal = holes_total / gridsVertical;
    while (holes_total % gridsVertical != 0) {
        gridsVertical--;
        gridsHorizontal = holes_total / gridsVertical;
    }

    int gridHeight = width / gridsVertical;
    int gridWidth = height / gridsHorizontal;

    int **random_coordinates = (int**)malloc(sizeof(int*) * holes_total);
    for (int i = 0; i < holes_total; i++) {
        random_coordinates[i] = (int *) malloc(sizeof(int) * 2);
    }

    int index = 0;

    // calculates random x and y coordinates within each grid that will serve as the center of the circle inside each grid
    for (int h = 0; h <= height - gridHeight; h += gridHeight) {
        for (int w = 0; w <= width - gridWidth; w += gridWidth) {
            int x_random = (int) h + rand() % gridHeight;
            int y_random = (int) w + rand() % gridWidth;
            random_coordinates[index][0] = x_random;
            random_coordinates[index][1] = y_random;
            index++;
        }
    }

    return random_coordinates;
}

void* apply_filters(void* arg) {
    struct filter_args* args = (struct filter_args*)arg;

    if (args->blur) {
        box_blur_filter(args);
    }
    if (args->cheese) {
        yellow_filter(args);
        draw_holes(args);
    }
    free(args);
    pthread_exit(NULL);
}

void process_threads(struct Pixel** pixels, struct DIB_Header DIB, struct BMP_Header BMP, bool blur, bool cheese, int** random_coordinates, int* holes_array, int holes_total) {
    pthread_t tids[THREAD_COUNT];
    struct thread_info** threads = (struct thread_info**)malloc(sizeof(struct thread_info*)*THREAD_COUNT);

    int thread_width = (DIB.width / THREAD_COUNT);

    int padding = 0;

    if (DIB.width % THREAD_COUNT != 0) {
        padding = DIB.width % THREAD_COUNT;
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        threads[i] = (struct thread_info*)malloc(sizeof(struct thread_info));
        threads[i]->start = 0;
        threads[i]->end = i == THREAD_COUNT - 1 ? thread_width + padding : thread_width;
    }

    struct Pixel*** tdata = (struct Pixel***)malloc(sizeof(struct Pixel**)*THREAD_COUNT);

    int start = 0;
    int end = thread_width;

    for (int i = 0; i < THREAD_COUNT; i++) {
        tdata[i] = (struct Pixel**)malloc(sizeof(struct Pixel*)*DIB.height);
        for (int j = 0; j < DIB.height; j++) {
            if (i == 0) { // cannot add extra column to the left so only adding extra to the right
                tdata[i][j] = (struct Pixel*)malloc(sizeof(struct Pixel)*(end + 2));
            }
            else if (i > 0 && i < THREAD_COUNT - 1) { // can add extra column to the left and extra to the right
                tdata[i][j] = (struct Pixel*)malloc(sizeof(struct Pixel)*(end + 4));
            }
            else { // cannot add extra column to the right so only adding extra to the left and any extra columns to the right if division of threads not even
                tdata[i][j] = (struct Pixel*)malloc(sizeof(struct Pixel)*(end + 2 + padding));
            }
        }
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        for (int h = 0; h < DIB.height; h++) {
            if (i == 0) {
                for (int w = start; w < end + 2; w++) {
                    tdata[i][h][w - start] = pixels[h][w];
                }
            }
            else if (i > 0 && i < THREAD_COUNT - 1) {
                for (int w = start; w < end + 4; w++) {
                    tdata[i][h][w - start] = pixels[h][w - 2];
                }
            }
            else {
                for (int w = start; w < end + 2 + padding ; w++) {
                    tdata[i][h][w - start] = pixels[h][w - 2];
                }
            }
        }
        start += thread_width;
        end += thread_width;
        threads[i]->data = tdata[i];
    }
    start = 0;
    end = thread_width;

    for (int i = 0; i < THREAD_COUNT; i++) {
        struct filter_args* args = (struct filter_args*)malloc(sizeof(struct filter_args));
        args->pArr = tdata[i];
        args->width = i == 0 ? thread_width + 2 : i > 0 && i < THREAD_COUNT - 1 ? thread_width + 4 : thread_width + 2 + padding;
        args->height = DIB.height;
        args->start_x = i == 0 ? start : start - 2;
        args->end_x = i == 0 ? end + 2 : i < THREAD_COUNT - 1 ? end + 2 : end + padding;
        args->coordinates = random_coordinates;
        args->radii = holes_array;
        args->holes_total = holes_total;
        args->blur = blur;
        args->cheese = cheese;
        pthread_create(&tids[i], NULL, apply_filters, args);

        start += thread_width;
        end += thread_width;
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(tids[i], NULL);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        start = i * thread_width;
        end = start + thread_width;

        if (i == THREAD_COUNT - 1) end = DIB.width;

        for (int h = 0; h < DIB.height; h++) {
            // have to account for extra columns added to the right of the first thread to blend pixels. need to write back into pixels normally since no left pad columns
            if (i == 0) {
                for (int w = start; w < end; w++) {
                    pixels[h][w] = tdata[i][h][w - start];
                }
            }
            // have to account for extra columns added to the left and right middle threads to blend pixels. need to write back into pixels starting at index 2 of tdata to write correct pixels
            else if (i > 0 && i < THREAD_COUNT - 1) {
                for (int w = start; w < end + 2; w++) {
                    pixels[h][w] = tdata[i][h][w - start + 2];
                }
            }
            // have to account for extra columns added to the left of the last thread to blend pixels. need to write back into pixels starting at index 2 of tdata to write correct pixels
            else {
                for (int w = start; w < DIB.width; w++) {
                    pixels[h][w] = tdata[i][h][w - start + 2];
                }
            }
        }
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        for (int h = 0; h < DIB.height; h++) {
            free(tdata[i][h]);
        }
        free(tdata[i]);
        free(threads[i]);
    }
    free(threads);
    free(tdata);
}

int main(int argc, char *argv[]) {
    int option;
    char *inputFile = NULL;
    char *outputFile = NULL;
    bool blur = false;
    bool cheese = false;

    if (argc != 7) {
        fprintf(stderr, "Usage: %s -i <input file> -o <output file> -f <filter>\n", argv[0]);
        return 1;
    }

    while ((option = getopt(argc, argv, "i:o:f:")) != -1) {
        switch (option) {
            case 'i':
                inputFile = optarg;
                break;
            case 'o':
                outputFile = optarg;
                break;
            case 'f':
                for (int i = 0; optarg[i] != '\0'; i++) {
                    if (optarg[i] == 'b') {
                        blur = true;
                    } else if (optarg[i] == 'c') {
                        cheese = true;
                    } else {
                        fprintf(stderr, "Invalid filter. Use 'b' for blur filter and 'c' for cheese filter.\n");
                        return 1;
                    }
                }
                break;
            case '?':
            default:
                fprintf(stderr, "Usage: %s -i <input file> -o <output file> -f <filter>\n", argv[0]);
                return 1;
        }
    }

    struct BMP_Header BMP;
    struct DIB_Header DIB;

    FILE *file_input = fopen(inputFile, "r");
    if (!file_input) {
        fprintf(stderr, "Error: Unable to open input file.\n");
        exit(EXIT_FAILURE);
    }
    readBMPHeader(file_input, &BMP);
    readDIBHeader(file_input, &DIB);

    struct Pixel **pixels = (struct Pixel **) malloc(sizeof(struct Pixel *) * DIB.height);
    for (int p = 0; p < DIB.height; p++) {
        pixels[p] = (struct Pixel *) malloc(sizeof(struct Pixel) * DIB.width);
    }

    readPixelsBMP(file_input, pixels, DIB.width, DIB.height);
    fclose(file_input);

    int holes_total = (int) fmin((double) DIB.width, (double) DIB.height) * 0.08;

    if (holes_total == 0) holes_total++;

    int* holes_array;
    holes_array = calculate_holes(pixels, DIB.height, DIB.width, holes_total);

    int** random_coordinates;
    random_coordinates = calculate_random_coordinates(DIB.height, DIB.width, holes_total);

    process_threads(pixels, DIB, BMP, blur, cheese, random_coordinates, holes_array, holes_total);

    FILE *file_output = fopen(outputFile, "wb");
    writeBMPHeader(file_output, &BMP);
    writeDIBHeader(file_output, &DIB);
    writePixelsBMP(file_output, pixels, DIB.width, DIB.height);
    fclose(file_output);

    for (int i = 0; i < holes_total; i++) {
        free(random_coordinates[i]);
    }
    free(random_coordinates);
    free(holes_array);

    for (int i = 0; i < DIB.height; i++) {
        free(pixels[i]);
    }
    free(pixels);

    return 0;
}
