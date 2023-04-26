#include "cachelab.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint;

const char ConstHelpString[] = "help string stub";
const int ConstM = 64;

typedef struct ProgramArgs
{
    bool Verbose;
    char TraceFilePath[256];
    int s, E, b;
} ProgramArgs;

ProgramArgs Args = {
    .Verbose = false, .TraceFilePath = {0}, .s = 0, .E = 0, .b = 0};

int ParseCommandLine(int argc, const char *argv[])
{
    for (int i = 0; i < argc; i++)
    {
        if (!strcmp(argv[i], "-h"))
        {
            printf(ConstHelpString);
            return 1;
        }
        if (!strcmp(argv[i], "-v"))
        {
            Args.Verbose = true;
        }
        if (!strcmp(argv[i], "-s"))
        {
            Args.s = atoi(argv[++i]);
        }
        if (!strcmp(argv[i], "-E"))
        {
            Args.E = atoi(argv[++i]);
        }
        if (!strcmp(argv[i], "-b"))
        {
            Args.b = atoi(argv[++i]);
        }
        if (!strcmp(argv[i], "-t"))
        {
            strcpy(Args.TraceFilePath, argv[++i]);
        }
    }
    return 0;
}

typedef struct Operation
{
    char OpType; // (I)NSTRUCT, (L)OAD, (S)TORE, (M)ODIFY
    uint Address;
    uint Size;
} Operation;

typedef struct Trace
{
    int Count;
    Operation Operations[1];
} Trace;

Trace *ParseTracefile()
{
    int tracesCount = 0;

    Trace *ret = NULL;
    FILE *file = fopen(Args.TraceFilePath, "r");
    char ch;

    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == '\n')
            tracesCount++;
    }

    ret = malloc(sizeof(Trace) + sizeof(Operation) * (tracesCount - 1));
    memset(ret, 0, sizeof(Trace) + sizeof(Operation) * (tracesCount - 1));
    fseek(file, 0, SEEK_SET);

    for (int i = 0; i < tracesCount; i++)
    {
        char line[256] = {0};
        // char ch = 0;
        // if ((ch = fgetc(file)) != ' ')
        //     fseek(file, -1, SEEK_CUR);
        fgets(line, 256, file);
        sscanf(line, " %c %x, %d", &ret->Operations[i].OpType,
               &ret->Operations[i].Address, &ret->Operations[i].Size);
    }

    fclose(file);
    ret->Count = tracesCount;
    return ret;
}

typedef struct CacheLine
{
    int TickSequence;
    bool Valid;
    uint Tag;
    char Block[256];
} CacheLine;

typedef struct Cache
{
    int s, E, b;
    int S, B, t, m;
    CacheLine Lines[1];
} Cache;

Cache *CacheInit(int s, int E, int b)
{
    Cache *ret = NULL;

    int S = (int)pow(2, (double)s);
    int B = (int)pow(2, (double)b);
    ret = malloc(sizeof(Cache) + sizeof(CacheLine) * (S - 1));
    memset(ret, 0, sizeof(Cache) + sizeof(CacheLine) * (S - 1));
    ret->s = s;
    ret->E = E;
    ret->b = b;
    ret->S = S;
    ret->B = B;
    const int m = ConstM;
    ret->m = m;
    ret->t = m - (s + b);

    return ret;
}

// [[gnu::always_inline]]
/* inline */ int CacheGetLineIndex(Cache *cache, int set, int line)
{
    return set * cache->E + line;
}

typedef struct CacheFetchResult
{
    bool Hit, Missed, Evicted;
} CacheFetchResult;

CacheFetchResult CacheFetch(Cache *cache, bool readAndWrite, uint address,
                            uint size, int ip)
{
    CacheFetchResult ret = {0};
    uint sIndex = (address >> cache->b) & ((1 << cache->s) - 1);
    uint tag = (address >> (cache->s + cache->b)) & ((1 << cache->t) - 1);

    int blankLineIndex = -1;
    int oldestLineIndex = CacheGetLineIndex(cache, sIndex, 0);
    for (int i = oldestLineIndex;
         i < CacheGetLineIndex(cache, sIndex, cache->E); i++)
    {
        if (cache->Lines[i].Valid && cache->Lines[i].Tag == tag)
        {
            ret.Hit = true;
            return ret;
        }

        if (!cache->Lines[i].Valid && blankLineIndex == -1)
            blankLineIndex = i;

        if (cache->Lines[i].Valid &&
            cache->Lines[i].TickSequence < cache->Lines[oldestLineIndex].TickSequence)
            oldestLineIndex = i;
    }

    CacheLine *line =
        &cache->Lines[blankLineIndex != -1 ? blankLineIndex : oldestLineIndex];
    line->TickSequence = ip;
    line->Valid = true;
    line->Tag = tag;
    memset(&line->Block[0], 0, size);

    ret.Missed = true;
    ret.Evicted = blankLineIndex == -1;
    return ret;
}

typedef struct SimulateResult
{
    int Hits, Misses, Evictions;
} SimulateResult;

SimulateResult Simulate(Cache *cache, Trace *trace)
{
    SimulateResult ret = {0};

    for (int ip = 0; ip < trace->Count; ip++)
    {
        Operation currentOp = trace->Operations[ip];
        CacheFetchResult fetchResult = {0}, secondResult = {0};
        switch (currentOp.OpType)
        {
        case 'I': // Instruction
            break;
        case 'L': // Load
            fetchResult =
                CacheFetch(cache, false, currentOp.Address, currentOp.Size, ip);
            break;
        case 'S': // Store
            fetchResult =
                CacheFetch(cache, true, currentOp.Address, currentOp.Size, ip);
            break;
        case 'M': // Modify
            fetchResult =
                CacheFetch(cache, false, currentOp.Address, currentOp.Size, ip);
            secondResult =
                CacheFetch(cache, true, currentOp.Address, currentOp.Size, ip);
            break;
        default:
            printf("err!");
            break;
        }

        ret.Hits += fetchResult.Hit ? 1 : 0;
        ret.Evictions += fetchResult.Evicted ? 1 : 0;
        ret.Misses += fetchResult.Missed ? 1 : 0;
        if (Args.Verbose)
            printf("%c %x,%d%s%s%s", currentOp.OpType, currentOp.Address,
                   currentOp.Size, fetchResult.Hit ? " hit" : "",
                   fetchResult.Missed ? " miss" : "",
                   fetchResult.Evicted ? " eviction" : "");

        if (currentOp.OpType == 'M')
        {
            ret.Hits += secondResult.Hit ? 1 : 0;
            ret.Evictions += secondResult.Evicted ? 1 : 0;
            ret.Misses += secondResult.Missed ? 1 : 0;
            if (Args.Verbose)
                printf("%s%s%s", secondResult.Hit ? " hit" : "",
                       secondResult.Missed ? " miss" : "",
                       secondResult.Evicted ? " eviction" : "");
        }

        if (Args.Verbose)
            printf("\n");
    }

    return ret;
}

int main(int argc, const char *argv[])
{
    if (ParseCommandLine(argc, argv))
        exit(0);

    Trace *trace = ParseTracefile();
    Cache *cache = CacheInit(Args.s, Args.E, Args.b);
    SimulateResult result = Simulate(cache, trace);
    printSummary(result.Hits, result.Misses, result.Evictions);

    return 0;
}
