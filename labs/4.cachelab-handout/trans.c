/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include "cachelab.h"
#include <stdio.h>

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    // hits:1765, misses:288, evictions:256
    if (M == 32 && N == 32)
    {
        for (int i = 0; i + 7 < N; i += 8)
        {
            for (int j = 0; j + 7 < M; j += 8)
            {
                for (int k = 0; k < 8; k++)
                {
                    int a1 = A[i + k][j + 0];
                    int a2 = A[i + k][j + 1];
                    int a3 = A[i + k][j + 2];
                    int a4 = A[i + k][j + 3];
                    int a5 = A[i + k][j + 4];
                    int a6 = A[i + k][j + 5];
                    int a7 = A[i + k][j + 6];
                    int a8 = A[i + k][j + 7];

                    B[j + 0][i + k] = a1;
                    B[j + 1][i + k] = a2;
                    B[j + 2][i + k] = a3;
                    B[j + 3][i + k] = a4;
                    B[j + 4][i + k] = a5;
                    B[j + 5][i + k] = a6;
                    B[j + 6][i + k] = a7;
                    B[j + 7][i + k] = a8;
                }
            }
        }
    }
    // hits:6497, misses:1700, evictions:1668
    else if (M == 64 && N == 64)
    {
        for (int i = 0; i + 3 < N; i += 4)
        {
            for (int j = 0; j + 3 < M; j += 4)
            {
                for (int k = 0; k < 4; k++)
                {
                    int a1 = A[i + k][j + 0];
                    int a2 = A[i + k][j + 1];
                    int a3 = A[i + k][j + 2];
                    int a4 = A[i + k][j + 3];

                    B[j + 0][i + k] = a1;
                    B[j + 1][i + k] = a2;
                    B[j + 2][i + k] = a3;
                    B[j + 3][i + k] = a4;
                }
            }
        }
    }
    // hits:6092, misses:2087, evictions:2055
    else if (M == 61 && N == 67)
    {
        int i;
        int j;
        for (i = 0; i + 7 < N; i += 8)
        {
            for (j = 0; j + 7 < M; j += 8)
            {
                for (int k = 0; k < 8; k++)
                {
                    int a1 = A[i + k][j + 0];
                    int a2 = A[i + k][j + 1];
                    int a3 = A[i + k][j + 2];
                    int a4 = A[i + k][j + 3];
                    int a5 = A[i + k][j + 4];
                    int a6 = A[i + k][j + 5];
                    int a7 = A[i + k][j + 6];
                    int a8 = A[i + k][j + 7];

                    B[j + 0][i + k] = a1;
                    B[j + 1][i + k] = a2;
                    B[j + 2][i + k] = a3;
                    B[j + 3][i + k] = a4;
                    B[j + 4][i + k] = a5;
                    B[j + 5][i + k] = a6;
                    B[j + 6][i + k] = a7;
                    B[j + 7][i + k] = a8;
                }
            }
        }
        for (int ii = i; ii < N; ii++)
        {
            for (int jj = 0; jj < M; jj++)
            {
                int tmp = A[ii][jj];
                B[jj][ii] = tmp;
            }
        }
        for (int ii = 0; ii < i; ii++)
        {
            for (int jj = j; jj < M; jj++)
            {
                int tmp = A[ii][jj];
                B[jj][ii] = tmp;
            }
        }
    }
    else
    {
        void trans(int M, int N, int A[N][M], int B[M][N]);
        trans(M, N, A, B);
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

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
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
    // registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}
