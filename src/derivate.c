#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <png.h>
#include <stdarg.h>

#include "../headers/utils.h"
#include "../headers/png_struct_manager.h"
#include "../headers/derivate.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

MatrixPNG matrix_dx(MatrixPNG png_matrix, Master_png master_png) {
    MatrixPNG png_matrix_copy = matrix_png_copy(png_matrix, master_png.main, master_png.info);
    for(int x = 0; x < png_matrix_copy->rows; x++) {
        for(int y = 0; y < png_matrix_copy->cols; y++)
            pixel_dx(png_matrix, png_pixel(png_matrix_copy, x, y), x, y);       
    }
    return png_matrix_copy;
}

MatrixPNG matrix_dy(MatrixPNG png_matrix, Master_png master_png) {
    MatrixPNG png_matrix_copy = matrix_png_copy(png_matrix, master_png.main, master_png.info);
    for(int x = 0; x < png_matrix_copy->rows; x++) {
        for(int y = 0; y < png_matrix_copy->cols; y++)
            pixel_dy(png_matrix, png_pixel(png_matrix_copy, x, y), x, y);
    }
    return png_matrix_copy;
}

png_bytep pixel_dx(MatrixPNG png_matrix, png_bytep derivate, int x, int y) {
    png_bytep MinusTwo, MinusOne, PlusOne, PlusTwo;

    MinusTwo = png_pixel(png_matrix, MIN(MAX(0, x - 2), png_matrix->rows - 1), y);
    MinusOne = png_pixel(png_matrix, MIN(MAX(0, x - 1), png_matrix->rows - 1), y);
    PlusOne = png_pixel(png_matrix, MIN(MAX(0, x + 1), png_matrix->rows - 1), y);
    PlusTwo = png_pixel(png_matrix, MIN(MAX(0, x + 2), png_matrix->rows - 1), y);

    derivate[0] = ( MinusTwo[0] - 8 * MinusOne[0] + 8 * PlusOne[0] - PlusTwo[0] ) / 12;
    derivate[1] = ( MinusTwo[1] - 8 * MinusOne[1] + 8 * PlusOne[1] - PlusTwo[1] ) / 12;
    derivate[2] = ( MinusTwo[2] - 8 * MinusOne[2] + 8 * PlusOne[2] - PlusTwo[2] ) / 12;
    derivate[3] = ( MinusTwo[3] - 8 * MinusOne[3] + 8 * PlusOne[3] - PlusTwo[3] ) / 12;

    return derivate;
}

png_bytep pixel_dy(MatrixPNG png_matrix, png_bytep derivate, int x, int y) {
    png_bytep MinusTwo, MinusOne, PlusOne, PlusTwo;

    MinusTwo = png_pixel(png_matrix, x, MIN(MAX(0, y - 2), png_matrix->cols - 1));
    MinusOne = png_pixel(png_matrix, x, MIN(MAX(0, y - 1), png_matrix->cols - 1));
    PlusOne = png_pixel(png_matrix, x, MIN(MAX(0, y + 1), png_matrix->cols - 1));
    PlusTwo = png_pixel(png_matrix, x, MIN(MAX(0, y + 2), png_matrix->cols - 1));

    derivate[0] = ( MinusTwo[0] - 8 * MinusOne[0] + 8 * PlusOne[0] - PlusTwo[0] ) / 12;
    derivate[1] = ( MinusTwo[1] - 8 * MinusOne[1] + 8 * PlusOne[1] - PlusTwo[1] ) / 12;
    derivate[2] = ( MinusTwo[2] - 8 * MinusOne[2] + 8 * PlusOne[2] - PlusTwo[2] ) / 12;
    derivate[3] = ( MinusTwo[3] - 8 * MinusOne[3] + 8 * PlusOne[3] - PlusTwo[3] ) / 12;

    return derivate;
}

png_bytep pixel_dt(MatrixPNG png_matrix, MatrixPNG png_matrix_two, png_bytep derivate, int x, int y) {
    derivate[0] = png_pixel(png_matrix, x, y)[0] - png_pixel(png_matrix_two, x, y)[0];
    derivate[1] = png_pixel(png_matrix, x, y)[1] - png_pixel(png_matrix_two, x, y)[1];
    derivate[2] = png_pixel(png_matrix, x, y)[2] - png_pixel(png_matrix_two, x, y)[2];
    derivate[3] = png_pixel(png_matrix, x, y)[3] - png_pixel(png_matrix_two, x, y)[3];
    return derivate;
}

MatrixPNG matrix_dt(Master_png master_png, MatrixPNG png_matrix, MatrixPNG png_matrix_two) {
    MatrixPNG png_matrix_copy = matrix_png_copy(png_matrix, master_png.main, master_png.info);
    for(int x = 0; x < png_matrix->rows; x++) {
        for(int y = 0; y < png_matrix->cols; y++) {  
            pixel_dt(png_matrix, png_matrix_two, png_pixel(png_matrix_copy, x, y), x, y);
        }
    }
    return png_matrix_copy;
}

void free_png_bytep(png_bytep px) {
    free(px);
}