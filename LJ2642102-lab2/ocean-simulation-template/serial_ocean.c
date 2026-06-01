#include <stdio.h>

void ocean (int **grid, int dim, int timesteps)
{
    /********************* the red-black algortihm (start)************************/
    /*
    In odd timesteps, calculate indeces with - and in even timesteps, calculate indeces with * 
    See the example of 6x6 matrix, A represents the corner elements. 
        A A A A A A
        A - * - * A
        A * - * - A
        A - * - * A
        A * - * - A
        A A A A A A 
    */

    // PUT YOUR CODE HERE
    int t, i, j;

    for (t = 0; t < timesteps; t++) {

        // ===== RED phase =====
        for (i = 1; i < dim - 1; i++) {
            int offset = (i + 1) % 2;
            for (j = 1 + offset; j < dim - 1; j += 2) {
                grid[i][j] = (
                    grid[i][j] +
                    grid[i][j-1] +
                    grid[i][j+1] +
                    grid[i-1][j] +
                    grid[i+1][j]
                ) / 5;
            }
        }

        // ===== BLACK phase =====
        for (i = 1; i < dim - 1; i++) {
            int offset = i % 2;
            for (j = 1 + offset; j < dim - 1; j += 2) {
                grid[i][j] = (
                    grid[i][j] +
                    grid[i][j-1] +
                    grid[i][j+1] +
                    grid[i-1][j] +
                    grid[i+1][j]
                ) / 5;
            }
        }
    }

}
