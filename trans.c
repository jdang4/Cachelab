// Jonathan Dang and Clay Oshiro-Leavitt
// WPI username: jdang@wpi.edu and coshiroleavitt@wpi.edu
// jdang-coshiroleavitt

/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

// function prototypes
int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void trans_32(int M, int N, int A[N][M], int B[M][N]);
void trans_64(int M, int N, int A[N][M], int B[M][N]);
void trans_Random(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {

    // determines which matrix transform helper function to use

    if (N == 32 && M == 32) {
        trans_32(M, N, A, B);
    }

    else if (N == 64 && M == 64) {
        trans_64(M, N, A, B);
    }

    else {
        trans_Random(M, N, A, B);
    }

}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * This is a transpose helper function used to optimize the cache to work for 32x32 array
 *
 * Jonathan Dang and Clay Oshiro-Leavitt
 */
char trans_32_desc[] = "32x32 scan transpose";
void trans_32 (int M, int N, int A[N][M], int B[M][N]) {

    // optimum block size for 32x32: 8
    int row, column;
    int r, c;
    int diagonalValue, location;

    // this iterates through the blocks and this increments by the size of the cache block
    for (row = 0; row < N; row += 8) {
        for (column = 0; column < M; column += 8) {
            // this iterates through each blocks
            for (r = row; r < row + 8 && r < N; r++) {
                for (c = column; c < column + 8 && c < M; c++) {

                    // checks to see if not a diagonal, so do the transpose
                    if (c != r) {
                        B[c][r] = A[r][c];
                    }

                    // if it is a diagonal
                    else {
                        diagonalValue = A[r][c]; // stores the value of the diagonal
                        location = r; // stores the location (index) of the diagonal
                    }
                }

                // check to see if it is a diagonal
                if (row == column) {
                    // sets the diagonal value at the transpose array
                    B[location][location] = diagonalValue;
                }
            }
        }
    }
}


/*
 * This is a transpose helper function intented to optimize the cache to work for a 64x64 array
 * (We were not able to fully optimize it, but still optimized to some degree)
 *
 * Jonathan Dang and Clay Oshiro-Leavitt
 */
char trans_64_desc[] = "64x64 scan transpose";
void trans_64 (int M, int N, int A[N][M], int B[M][N]) {


    // optimal block size for 64x64: 4
    int row, column;
    int r, c;
    int diagonalValue, location;

    // this iterates through the blocks, this increments by the cache block size
    for (row = 0; row < N; row += 4) {
        for (column = 0; column < M; column += 4) {
            // this iterates through each blocks
            for (r = row; r < row + 4 && r < N; r++) {
                for (c = column; c < column + 4 && c < M; c++) {

                    // checks to see if not a diagonal, so do the transpose
                    if (c != r) {
                        B[c][r] = A[r][c];
                    }

                    // if it is a diagonal
                    else {
                        diagonalValue = A[r][c]; // stores the value of the diagonal
                        location = r; // stores the location (index) of the diagonal
                    }
                }

                // check to see if it is a diagonal
                if (row == column) {
                    // sets the diagonal value at the transpose array
                    B[location][location] = diagonalValue;
                }
            }
        }
    }
}

/*
 * This is a transpose helper function that is optimized for the cache to work for a 61x67 array and any other
 * arbitrary matrices of ~4096 elements
 *
 * Jonathan Dang and Clay Oshiro-Leavitt
 */
char trans_Random_desc[] = "MxN scan transpose";
void trans_Random (int M, int N, int A[N][M], int B[M][N]) {

    // optimum block size: 16

    int row, column;
    int r, c;
    int diagonalValue, location;

    // this iterates through the blocks, this increments by the cache block size
    for (row = 0; row < N; row += 16) {
        for (column = 0; column < M; column += 16) {
            // this iterates through each blocks
            for (r = row; r < row + 16 && r < N; r++) {
                for (c = column; c < column + 16 && c < M; c++) {

                    // checks to see if not a diagonal, so do the transpose
                    if (c != r) {
                        B[c][r] = A[r][c];
                    }

                    // if it is a diagonal
                    else {
                        diagonalValue = A[r][c]; // stores the value of the diagonal
                        location = r; // stores the location (index) of the diagonal
                    }
                }

                // check to see if it is a diagonal
                if (row == column) {
                    // sets the diagonal value at the transpose array
                    B[location][location] = diagonalValue;
                }
            }
        }
    }
}

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);

    // my additional transpose functions
    registerTransFunction(trans_32, trans_32_desc);
    registerTransFunction(trans_64, trans_64_desc);
    registerTransFunction(trans_Random, trans_Random_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
