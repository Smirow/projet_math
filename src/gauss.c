#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <png.h>

#include "../headers/utils.h"
#include "../headers/png_struct_manager.h"
#include "../headers/gauss.h"

Matrix_filter gauss_filter(int sigma, int mu) {
    int x, y;
    Matrix_filter filter =  (Matrix_filter) malloc(sizeof(Matrix_filter));
    filter->rows = mu;
    filter->cols = mu;
    filter->mat = (double**) calloc(mu, sizeof(double*));
    for (int i = 0; i < mu; i++)
        filter->mat[i] = (double*) calloc(mu, sizeof(double));

    for (int i = 0; i < mu; i++) {
        for (int j = 0; j < mu; j++) {
            x = j - mu/2;
            y = i - mu/2;
            filter->mat[i][j] = (double) (1. / (2. * M_PI * sigma * sigma)) * exp(-(x * x + y * y) / (2* sigma * sigma));
        }
    }
    return filter;
}


void print_matrix_filter(Matrix_filter matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++)
            printf("%f ", matrix->mat[i][j]);
        
        printf("\n");
    }
}

MatrixPNG gauss_blur(MatrixPNG png_matrix, Master_png master_png, int sigma, int mu) {
    png_bytep px;
    int current_x, current_y;
    double resR = 0.0, resG = 0.0, resB = 0.0, resA = 0.0, sum = 0.0;
    MatrixPNG png_matrix_copy = matrix_png_copy(png_matrix, master_png.main, master_png.info);
    Matrix_filter filter_matrix = gauss_filter(sigma, mu);
    for(int x = 0; x < png_matrix->rows; x++) {
        for(int y = 0; y < png_matrix->cols; y++) {
            for (int i = - mu/2; i <= mu/2; i++) {
                for (int j = - mu/2; j <= mu/2; j++) {
                    current_x = x + i;
                    current_y = y + j;

                    if (x + i < 0)
                        current_x = 0;
                    if (y + j < 0)
                        current_y = 0;
                    if (x + i >= png_matrix->rows)
                        current_x = png_matrix->rows - 1;
                    if (y + j >= png_matrix->cols)
                        current_y = png_matrix->cols - 1;

                    px = png_pixel(png_matrix, current_x, current_y);

                    sum += filter_matrix->mat[i + mu/2][j + mu/2];
                    resR += filter_matrix->mat[i + mu/2][j + mu/2] * px[0];
                    resG += filter_matrix->mat[i + mu/2][j + mu/2] * px[1];
                    resB += filter_matrix->mat[i + mu/2][j + mu/2] * px[2];
                    resA += filter_matrix->mat[i + mu/2][j + mu/2] * px[3];
                }
            }
            px = png_pixel(png_matrix_copy, x, y);

            px[0] = resR / sum;
            px[1] = resG / sum;
            px[2] = resB / sum;
            px[3] = resA / sum;
            
            resR = 0.0, resG = 0.0, resB = 0.0, resA = 0.0, sum = 0.0;
        }
    }
    return png_matrix_copy;
}