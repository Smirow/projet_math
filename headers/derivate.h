/* ABOU ALI Magued, Avril 2017 */

MatrixPNG matrix_dx(MatrixPNG png_matrix, Master_png master_png);
MatrixPNG matrix_dy(MatrixPNG png_matrix, Master_png master_png);
png_bytep pixel_dx(MatrixPNG png_matrix, png_bytep derivate, int x, int y);
png_bytep pixel_dy(MatrixPNG png_matrix, png_bytep derivate, int x, int y);
png_bytep pixel_dt(MatrixPNG png_matrix, MatrixPNG png_matrix_two, png_bytep derivate, int x, int y);
MatrixPNG matrix_dt(Master_png master_png, MatrixPNG png_matrix, MatrixPNG png_matrix_two);