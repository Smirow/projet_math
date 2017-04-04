
#define PNG_DEBUG 3
#ifndef M_PI
 #define M_PI 3.14
#endif 

Matrix_filter gauss_filter(int sigma, int mu);
void print_matrix_filter(Matrix_filter matrix, int rows, int cols);
MatrixPNG gauss_blur(MatrixPNG png_matrix, Master_png master_png, int sigma, int mu);