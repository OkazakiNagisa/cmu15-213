#include "cachelab.h"
#include <stdio.h>

extern void randMatrix(int M, int N, int A[N][M]);

extern int is_transpose(int M, int N, int A[N][M], int B[M][N]);
extern void transpose_submit(int M, int N, int A[N][M], int B[M][N]);
extern void trans(int M, int N, int A[N][M], int B[M][N]);

int main(int argc, const char *argv[])
{
    int A[67][61] = {0};
    int B[61][67] = {0};
    // randMatrix(32, 32, A);
    int a = 0;
    for (int i = 0; i < 67; i++)
    {
        for (int j = 0; j < 61; j++)
        {
            A[i][j] = ++a;
        }
    }

    // trans(32, 32, A, B);
    transpose_submit(61, 67, A, B);

    printf("%s\n", is_transpose(61, 67, A, B) ? "true" : "false");
    return 0;
}