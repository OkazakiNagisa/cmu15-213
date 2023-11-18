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

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    for (int i = 0; i + 3 < N; i += 4)
    {
        for (int j = 0; j + 3 < M; j += 4)
        {
            int a11 = A[i + 0][j + 0];
            int a12 = A[i + 0][j + 1];
            int a13 = A[i + 0][j + 2];
            int a14 = A[i + 0][j + 3];

            int a21 = A[i + 1][j + 0];
            int a22 = A[i + 1][j + 1];
            int a23 = A[i + 1][j + 2];
            int a24 = A[i + 1][j + 3];

            B[j + 0][i + 0] = a11;
            B[j + 0][i + 1] = a21;

            B[j + 1][i + 0] = a12;
            B[j + 1][i + 1] = a22;

            B[j + 2][i + 0] = a13;
            B[j + 2][i + 1] = a23;

            B[j + 3][i + 0] = a14;
            B[j + 3][i + 1] = a24;

            a11 = A[i + 2][j + 0];
            a12 = A[i + 2][j + 1];
            a13 = A[i + 2][j + 2];
            a14 = A[i + 2][j + 3];

            a21 = A[i + 3][j + 0];
            a22 = A[i + 3][j + 1];
            a23 = A[i + 3][j + 2];
            a24 = A[i + 3][j + 3];

            B[j + 0][i + 2] = a11;
            B[j + 0][i + 3] = a21;

            B[j + 1][i + 2] = a12;
            B[j + 1][i + 3] = a22;

            B[j + 2][i + 2] = a13;
            B[j + 2][i + 3] = a23;

            B[j + 3][i + 2] = a14;
            B[j + 3][i + 3] = a24;
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
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
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
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

