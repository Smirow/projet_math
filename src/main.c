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
#include "../headers/derivate.h"


int main(int argc, char **argv) {
    if (argc != 3)
        abort_("usage: ./a.out png_source_file png_dest_filename");

    FILE *fp = fopen(argv[1], "rb");
    if (!fp)
        abort_("[read_png_file] File %s could not be opened for reading", argv[1]);

    FILE *fp_two = fopen(argv[2], "rb");
    if (!fp_two)
        abort_("[read_png_file] File %s could not be opened for reading", argv[2]);

    Master_png master_png = init_master_png(fp);
    MatrixPNG matrix = initPngMatrix(master_png.main, master_png.info);
    printf("W: %d, H: %d\n", matrix->cols, matrix->rows);

    Master_png master_png_two = init_master_png(fp_two);
    MatrixPNG matrix_two = initPngMatrix(master_png_two.main, master_png_two.info);
    printf("W: %d, H: %d\n", matrix->cols, matrix->rows);

    // Gauss Blur
    MatrixPNG matrix_copy = gauss_blur(matrix, master_png, 4, 9);
    MatrixPNG matrix_copy_two = gauss_blur(matrix_two, master_png_two, 4, 9);
    
    write_png_file("Venus10_Gauss_Blur.png",  matrix_copy);
    write_png_file("Venus11_Gauss_Blur.png",  matrix_copy_two);

    // Derivate
    matrix_dx(matrix, master_png);
    matrix_dy(matrix, master_png);
    matrix_dt(master_png, matrix_copy, matrix_copy_two);

    fclose(fp);

    return 0;
}