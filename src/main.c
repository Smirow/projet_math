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


int main(int argc, char **argv) {
    if (argc != 3)
        abort_("usage: ./a.out png_source_file png_dest_filename");

    FILE *fp = fopen(argv[1], "rb");
    if (!fp)
        abort_("[read_png_file] File %s could not be opened for reading", argv[1]);
    
    Master_png master_png = init_master_png(fp);
    MatrixPNG matrix = initPngMatrix(master_png.main, master_png.info);
    printf("W: %d, H: %d\n", matrix->cols, matrix->rows);

    MatrixPNG matrix_copy = gauss_blur(matrix, master_png, 4, 7);
    write_png_file(argv[2],  matrix_copy);

    fclose(fp);

    return 0;
}