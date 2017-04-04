#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <png.h>
#include <stdarg.h>


#include "../headers/utils.h"
#include "../headers/png_struct_manager.h"

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

MatrixPNG matrix_png_copy(MatrixPNG png_matrix, png_structp png_ptr, png_infop info_ptr) {
    MatrixPNG png_matrix_copy = (MatrixPNG) malloc(sizeof(MatrixPNG));
    png_matrix_copy->rows = png_matrix->rows;
    png_matrix_copy->cols = png_matrix->cols;

    png_matrix_copy->mat = (png_bytep*) malloc(sizeof(png_bytep) * png_matrix_copy->cols);
    for(int y = 0; y < png_matrix_copy->cols; y++)
        png_matrix_copy->mat[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr, info_ptr));

    for(int x = 0; x < png_matrix_copy->rows; x++) {
        for(int y = 0; y < png_matrix_copy->cols * 4; y++)
            png_matrix_copy->mat[x][y] = png_matrix->mat[x][y];
    }
    return png_matrix_copy;
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