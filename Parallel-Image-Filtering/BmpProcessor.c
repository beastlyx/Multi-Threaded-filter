/**
* Implementation of several functions to manipulate BMP files.
*
* Completion time: 35 hours
*
* @author Borys Banaszkiewicz, Ruben Acuna
* @version 01/22/2024
*/

#include "BmpProcessor.h"
#include <string.h>

/**
 * Read BMP header of a BMP file.
 *
 * @param  file: A pointer to the file being read
 * @param  header: Pointer to the destination BMP header
 */
void readBMPHeader(FILE* file, struct BMP_Header* header) {
    fseek(file, 0, SEEK_SET);
    fread(&header->signature, sizeof(char) * 2, 1, file);
    fread(&header->size, sizeof(int), 1, file);
    fread(&header->reserved1, sizeof(short), 1, file);
    fread(&header->reserved2, sizeof(short), 1, file);
    fread(&header->offset_pixel_array, sizeof(int), 1, file);
}

/**
 * Write BMP header of a file. Useful for creating a BMP file.
 *
 * @param  file: A pointer to the file being written
 * @param  header: The header to write to the file
 */
void writeBMPHeader(FILE* file, struct BMP_Header* header) {
    fwrite(&header->signature, sizeof(char) * 2, 1, file);
    fwrite(&header->size, sizeof(int), 1, file);
    fwrite(&header->reserved1, sizeof(short), 1, file);
    fwrite(&header->reserved2, sizeof(short), 1, file);
    fwrite(&header->offset_pixel_array, sizeof(int), 1, file);
}

/**
 * Read DIB header from a BMP file.
 *
 * @param  file: A pointer to the file being read
 * @param  header: Pointer to the destination DIB header
 */
void readDIBHeader(FILE* file, struct DIB_Header* header) {
    fseek(file, 14, SEEK_SET);
    fread(&header->size, sizeof(int), 1, file);
    fread(&header->width, sizeof(int), 1, file);
    fread(&header->height, sizeof(int), 1, file);
    fread(&header->planes, sizeof(short), 1, file);
    fread(&header->bitsPerPixel, sizeof(short), 1, file);
    fread(&header->compression, sizeof(int), 1, file);
    fread(&header->imageSize, sizeof(int), 1, file);
    fread(&header->horizRes, sizeof(int), 1, file);
    fread(&header->vertRes, sizeof(int), 1, file);
    fread(&header->colorNum, sizeof(int), 1, file);
    fread(&header->importantColorNum, sizeof(int), 1, file);
}

/**
 * Write DIB header of a file. Useful for creating a BMP file.
 *
 * @param  file: A pointer to the file being written
 * @param  header: The header to write to the file
 */
void writeDIBHeader(FILE* file, struct DIB_Header* header) {
    fwrite(&header->size, sizeof(int), 1, file);
    fwrite(&header->width, sizeof(int), 1, file);
    fwrite(&header->height, sizeof(int), 1, file);
    fwrite(&header->planes, sizeof(short), 1, file);
    fwrite(&header->bitsPerPixel, sizeof(short), 1, file);
    fwrite(&header->compression, sizeof(int), 1, file);
    fwrite(&header->imageSize, sizeof(int), 1, file);
    fwrite(&header->horizRes, sizeof(int), 1, file);
    fwrite(&header->vertRes, sizeof(int), 1, file);
    fwrite(&header->colorNum, sizeof(int), 1, file);
    fwrite(&header->importantColorNum, sizeof(int), 1, file);
}

/**
 * Make BMP header based on width and height. Useful for creating a BMP file.
 *
 * @param  header: Pointer to the destination DIB header
 * @param  width: Width of the image that this header is for
 * @param  height: Height of the image that this header is for
 */
void makeBMPHeader(struct BMP_Header* header, int width, int height) {
    strcpy(header->signature, "BM");
    int row_size = (width * 3 + 3) & ~3;
    header->size = 54 + row_size * height;
    header->reserved1 = 0;
    header->reserved2 = 0;
    header->offset_pixel_array = 54;
}

/**
* Make new DIB header based on width and height.Useful for creating a BMP file.
*
* @param  header: Pointer to the destination DIB header
* @param  width: Width of the image that this header is for
* @param  height: Height of the image that this header is for
*/
void makeDIBHeader(struct DIB_Header* header, int width, int height) {
    header->size = 40;
    header->width = width;
    header->height = height;
    header->planes = 1;
    header->bitsPerPixel = 24;
    header->compression = 0;
    int row_size = (width * 3 + 3) & ~3;
    header->imageSize = row_size * height; // have to add padding
    header->horizRes = 3780;
    header->vertRes = 3780;
    header->colorNum = 0;
    header->importantColorNum = 0;
}

/**
 * Read Pixels from BMP file based on width and height.
 *
 * @param  file: A pointer to the file being read
 * @param  pArr: Pixel array to store the pixels being read
 * @param  width: Width of the pixel array of this image
 * @param  height: Height of the pixel array of this image
 */
void readPixelsBMP(FILE* file, struct Pixel** pArr, int width, int height) {
    fseek(file, 54, SEEK_SET);

    int padding = (4 - (width * (int)sizeof(struct Pixel)) % 4) % 4;

    for (int i = height - 1; i >= 0; i--) {
        fread(pArr[i], sizeof(struct Pixel), width, file);
        fseek(file, padding, SEEK_CUR);
    }
}

/**
 * Write Pixels from BMP file based on width and height.
 *
 * @param  file: A pointer to the file being read or written
 * @param  pArr: Pixel array of the image to write to the file
 * @param  width: Width of the pixel array of this image
 * @param  height: Height of the pixel array of this image
 */
void writePixelsBMP(FILE* file, struct Pixel** pArr, int width, int height) {
    int padding = (4 - (width * (int)sizeof(struct Pixel)) % 4) % 4;

    unsigned char pad[3] = {0,0,0};

    for (int i = height - 1; i >= 0; i--) {
        fwrite(pArr[i], sizeof(struct Pixel), width, file);

        if (padding > 0) {
            fwrite(pad, 1, padding, file);
        }
    }
}