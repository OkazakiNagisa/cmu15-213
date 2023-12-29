/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "lct",
    /* First member's full name */
    "nakido",
    /* First member's email address */
    "nakido@seu.edu.cn",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define HDRSIZE 4
#define CHUNKSIZE (1 << 12)

typedef union
{
    struct
    {
        bool allocated : 1;
        bool unused1 : 1;
        bool unused2 : 1;
        uint32_t blocksize : 29;
    };
    uint32_t data;
} header_t, footer_t;

static char *heap_start;

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    heap_start = mem_sbrk(CHUNKSIZE);
    if (heap_start == (void *)-1)
        return -1;

    if ((uint64_t)heap_start & (ALIGNMENT - 1))
        heap_start = (void *)(((uint64_t)heap_start & ~(ALIGNMENT - 1)) + ALIGNMENT);

    ((header_t *)heap_start)->data = 0;
    ((header_t *)heap_start)->allocated = true;
    ((footer_t *)heap_start + 1)->data = 0;
    ((footer_t *)heap_start + 1)->allocated = true;

    header_t *epilogue = (header_t *)(heap_start + CHUNKSIZE - sizeof(header_t) * 2);

    ((header_t *)epilogue)->data = 0;
    ((header_t *)epilogue)->allocated = true;
    ((footer_t *)epilogue + 1)->data = 0;
    ((footer_t *)epilogue + 1)->allocated = true;
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
        return NULL;
    else
    {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

int32_t mm_check()
{
    return 0;
}