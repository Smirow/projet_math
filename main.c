#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#define PNG_DEBUG 3
#ifndef M_PI
 #define M_PI 3.14
#endif 
#include <png.h>

typedef struct struct_matrix_filter {
    double** mat;
    int rows, cols;
} *Matrix_filter;

typedef struct struct_matrix_png {
    png_bytep* mat;
    int rows, cols;
} *MatrixPNG;

typedef struct struct_png_master {
    png_structp main;
    png_infop info;
} Master_png;

void abort_(const char * s, ...) {
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

png_structp pngToPng_Structp(FILE *fp) {
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
        abort_("[read_png_file] png_create_read_struct failed");
        
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[read_png_file] Error during init_io");

    png_init_io(png_ptr, fp);

    return png_ptr;
}

png_infop png_inf(png_structp png_ptr) {
    png_infop info_ptr = png_create_info_struct(png_ptr);
    png_read_info(png_ptr, info_ptr);
    if (!info_ptr)
        abort_("[read_png_file] png_create_info_struct failed");
        
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth  = png_get_bit_depth(png_ptr, info_ptr);

    if(bit_depth == 16)
        png_set_strip_16(png_ptr);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);

    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

  // These color_type don't have an alpha channel then fill it with 0xff.
    if(color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);

    if(color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    return info_ptr;
}

Master_png init_master_png(FILE *fp) {
    Master_png master;
    master.main = pngToPng_Structp(fp);
    master.info = png_inf(master.main);
    return master;
} 

int png_width(png_structp png_ptr, png_infop info_ptr) {    
    return png_get_image_width(png_ptr, info_ptr);
}

int png_height(png_structp png_ptr, png_infop info_ptr) {
    return png_get_image_height(png_ptr, info_ptr);
}

MatrixPNG initPngMatrix(png_structp png_ptr, png_infop info_ptr) {
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[read_png_file] Error during read_image");
    int height = png_height(png_ptr, info_ptr);
    int width = png_width(png_ptr, info_ptr);
    MatrixPNG matrix = (MatrixPNG) malloc(sizeof(MatrixPNG));
    matrix->rows = height;
    matrix->cols = width;

    matrix->mat = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for(int y = 0; y < height; y++)
        matrix->mat[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr, info_ptr));
    
    png_read_image(png_ptr, matrix->mat);
    return matrix;
}

png_bytep png_pixel(MatrixPNG matrix, int x, int y) {
    return &(matrix->mat[x][y * 4]);
}

Matrix_filter gauss_filter(int sigma, int mu) {
    int x, y;
    Matrix_filter filter =  (Matrix_filter) malloc(sizeof(Matrix_filter));
    filter->rows = mu;
    filter->cols = mu;
    filter->mat = (double**) calloc(mu, sizeof(double*));
    for (int i = 0; i < mu; i++) {
        filter->mat[i] = (double*) calloc(mu, sizeof(double));
    }
    for (int i = 0; i < mu; i++) {
        for (int j = 0; j < mu; j++) {
            x = j - mu/2;
            y = i - mu/2;
            filter->mat[i][j] = (double) (1. / (2. * M_PI * sigma * sigma)) * exp(-(x * x + y * y) / (2* sigma * sigma));
        }
    }
    return filter;
}

MatrixPNG matrix_png_copy(MatrixPNG png_matrix, png_structp png_ptr, png_infop info_ptr) {
    MatrixPNG png_matrix_copy = (MatrixPNG) malloc(sizeof(MatrixPNG));
    png_matrix_copy->rows = png_matrix->rows;
    png_matrix_copy->cols = png_matrix->cols;

    png_matrix_copy->mat = (png_bytep*) malloc(sizeof(png_bytep) * png_matrix_copy->cols);
    for(int y = 0; y < png_matrix_copy->cols; y++)
        png_matrix_copy->mat[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr, info_ptr));

    for(int x = 0; x < png_matrix_copy->rows; x++) {
        for(int y = 0; y < png_matrix_copy->cols * 4; y++) {
            png_matrix_copy->mat[x][y] = png_matrix->mat[x][y];
         }
    }
    return png_matrix_copy;
}

void gauss_blur(MatrixPNG png_matrix, MatrixPNG png_matrix_copy, int sigma, int mu) {
    png_bytep px;
    int current_x, current_y;
    double resR = 0.0, resG = 0.0, resB = 0.0, resA = 0.0, sum = 0.0;
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
}

void write_png_file(char* filename, MatrixPNG matrix) {
    FILE *fp = fopen(filename, "wb");
        if(!fp) abort();

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png) abort();
    
    png_infop info = png_create_info_struct(png);
        if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_set_IHDR(
    png,
    info,
    matrix->cols, matrix->rows,
    8,
    PNG_COLOR_TYPE_RGBA,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
    );
    
    
    png_write_info(png, info);

    png_write_image(png, matrix->mat);
    png_write_end(png, NULL);

    fclose(fp);
}


void print_matrix(Matrix_filter matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%f ", matrix->mat[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    FILE *fp = fopen(argv[1], "rb");
    if (!fp)
        abort_("[read_png_file] File %s could not be opened for reading", argv[1]);
    Master_png master_png = init_master_png(fp);
    MatrixPNG matrix = initPngMatrix(master_png.main, master_png.info);
    MatrixPNG matrix_copy = matrix_png_copy(matrix, master_png.main, master_png.info);
    printf("W: %d, H: %d\n", matrix->cols, matrix->rows);
    // for(int x = 0; x < matrix_copy->rows; x++) {
    //     for(int y = 0; y < matrix_copy->cols; y++) {
    //         png_bytep px = png_pixel(matrix_copy, x, y);
    //         px[0] = 100;
    //         printf("[ %d - %d ] = RGBA(%3d, %3d, %3d, %3d)\n", x + 1, y + 1, px[0], px[1], px[2], px[3]);
    //     }
    // }
    gauss_blur(matrix, matrix_copy, 4, 7);
    write_png_file("out.png", matrix_copy);
    fclose(fp);
    return 0;
}