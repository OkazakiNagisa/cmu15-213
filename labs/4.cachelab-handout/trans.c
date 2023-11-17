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
    // void trans(int M, int N, int A[N][M], int B[M][N]);
    // trans(M, N, A, B);
    // hits:869, misses:1184, evictions:1152

    // for (int i = 0; i + 3 < N; i += 3)
    // {
    //     for (int j = 0; j + 3 < N; j += 3)
    //     {
    //         int a1 = A[i][j];
    //         int a2 = A[i][j + 1];
    //         int a3 = A[i][j + 2];
    //         int a4 = A[i + 1][j];
    //         int a5 = A[i + 1][j + 1];
    //         int a6 = A[i + 1][j + 2];
    //         int a7 = A[i + 2][j];
    //         int a8 = A[i + 2][j + 1];
    //         int a9 = A[i + 2][j + 2];
    //         B[j][i] = a1;
    //         B[j][i + 1] = a4;
    //         B[j][i + 2] = a7;
    //         B[j + 1][i] = a2;
    //         B[j + 1][i + 1] = a5;
    //         B[j + 1][i + 2] = a8;
    //         B[j + 2][i] = a3;
    //         B[j + 2][i + 1] = a6;
    //         B[j + 2][i + 2] = a9;
    //     }
    // }
    for (int i = 0; i + 4 < N; i += 4)
    {
        for (int j = 0; j + 4 < N; j += 4)
        {
            int a11 = A[i + 0][j + 0];
            int a12 = A[i + 0][j + 1];
            int a13 = A[i + 0][j + 2];
            int a14 = A[i + 0][j + 3];

            int a21 = A[i + 1][j + 0];
            int a22 = A[i + 1][j + 1];
            int a23 = A[i + 1][j + 2];
            int a24 = A[i + 1][j + 3];

            int a31 = A[i + 2][j + 0];
            int a32 = A[i + 2][j + 1];
            int a33 = A[i + 2][j + 2];
            int a34 = A[i + 2][j + 3];

            int a41 = A[i + 3][j + 0];
            int a42 = A[i + 3][j + 1];
            int a43 = A[i + 3][j + 2];
            int a44 = A[i + 3][j + 3];

            B[j + 0][i + 0] = a11;
            B[j + 0][i + 1] = a21;
            B[j + 0][i + 2] = a31;
            B[j + 0][i + 3] = a41;

            B[j + 1][i + 0] = a12;
            B[j + 1][i + 1] = a22;
            B[j + 1][i + 2] = a32;
            B[j + 1][i + 3] = a42;

            B[j + 2][i + 0] = a13;
            B[j + 2][i + 1] = a23;
            B[j + 2][i + 2] = a33;
            B[j + 2][i + 3] = a43;

            B[j + 3][i + 0] = a14;
            B[j + 3][i + 1] = a24;
            B[j + 3][i + 2] = a34;
            B[j + 3][i + 3] = a44;
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

