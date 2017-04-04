

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

png_structp pngToPng_Structp(FILE *fp);
png_infop png_inf(png_structp png_ptr);
Master_png init_master_png(FILE *fp);
int png_width(png_structp png_ptr, png_infop info_ptr);
int png_height(png_structp png_ptr, png_infop info_ptr);
MatrixPNG initPngMatrix(png_structp png_ptr, png_infop info_ptr);
png_bytep png_pixel(MatrixPNG matrix, int x, int y);
MatrixPNG matrix_png_copy(MatrixPNG png_matrix, png_structp png_ptr, png_infop info_ptr);
void write_png_file(char* filename, MatrixPNG matrix);