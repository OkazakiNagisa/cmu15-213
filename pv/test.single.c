#include <stdio.h>
#include <stdbool.h>

long long trueFiveEighths(long long x)
{
    return (x >> 1) + (x >> 3) + ((x >> 63) & 1) - !(x ^ (1ll << 63));
}

long long addOK(long long x, long long y)
{
    return ((~((x | y) >> 63) & (x + y) >> 63) & 1) + !!((x ^ y) >> 63) +
           ((((x & y) >> 63) & (x + y) >> 63) & 1);
}

long long isPower2(long long x)
{
    return ((x ^ 1) & ~(x >> 63) & 1) - !(x ^ 0);
}

long long rotateLeft(long long x, long long n)
{
    return (x << n) | ((x >> (64 - n)) & ((1ll << n) - 1));
}

int main(int argc, const char *argv[])
{
    long long x = 0x8000000000000000;
    long long y = 0x7fffffffffffffff;
    long long n = 32;
    long long ans = 0x80000000;

    bool right = (rotateLeft(x, n) == ans);

    printf("%s", right ? "true" : "false");
    return 0;
}