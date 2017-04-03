#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define PNG_DEBUG 3
#include <png.h>

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
    png_set_sig_bytes(png_ptr, 0);

    return png_ptr;
}

png_infop png_inf(png_structp png_ptr) {
    png_infop info_ptr = png_create_info_struct(png_ptr);
    png_read_info(png_ptr, info_ptr);
    if (!info_ptr)
        abort_("[read_png_file] png_create_info_struct failed");
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

int main(int argc, char **argv) {
    FILE *fp = fopen(argv[1], "rb");
    if (!fp)
        abort_("[read_png_file] File %s could not be opened for reading", argv[1]);
    Master_png master_png = init_master_png(fp);
    // png_structp png = pngToPng_Structp(fp);
    // png_infop png_in = png_inf(png);
    MatrixPNG matrix = initPngMatrix(master_png.main, master_png.info);
    printf("W: %d, H: %d\n", matrix->cols, matrix->rows);
    for(int x = 0; x < matrix->rows; x++) {
        for(int y = 0; y < matrix->cols; y++) {
            png_bytep px = png_pixel(matrix, x, y);
            printf("[ %d - %d ] = RGBA(%3d, %3d, %3d, %3d)\n", x + 1, y + 1, px[0], px[1], px[2], px[3]);
        }
    }
    fclose(fp);
    return 0;
}