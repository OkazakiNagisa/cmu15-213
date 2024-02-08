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

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define CHUNKSIZE (1 << 12)

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size_t)(size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

typedef union
{
    struct
    {
        bool allocated : 1;
        bool unused1 : 1;
        bool unused2 : 1;
        // uint32_t blocksize : 29;
    };
    uint32_t data;
} Header, Footer;

typedef struct
{
    Header header;
    unsigned char memory[1];
} Block;

typedef union
{
    size_t as_int;
    void *as_ptr;
    union
    {
        Header *asHeader;
        Footer *asFooter;
    };
} Ptr;

static void *true_heap_start;
static void *heap_start; // aligned
static void *heap_end;

Ptr getPrologue()
{
    Ptr ret;
    ret.as_ptr = heap_start;
    return ret;
}

Ptr getEpilogue()
{
    Ptr ret;
    ret.as_ptr = heap_end;
    ret.asHeader -= 2;
    return ret;
}

uint32_t getHeaderBlocksize(Header *header)
{
    return header->data & ~0b111;
}

Footer *getFooterFromHeader(Header *header)
{
    Ptr ret;
    ret.as_ptr = header;
    uint32_t blockSize = getHeaderBlocksize(header);
    ret.as_int += sizeof(Header) + blockSize;
    return ret.asFooter;
}

Header *getHeaderFromFooter(Footer *footer)
{
    Ptr ret;
    ret.as_ptr = footer;
    uint32_t blockSize = getHeaderBlocksize(footer);
    ret.as_int -= sizeof(Footer) + blockSize;
    return ret.asHeader;
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    true_heap_start = mem_sbrk(CHUNKSIZE);
    if (true_heap_start == (void *)-1)
        return -1;
    heap_start = (void *)ALIGN((size_t)true_heap_start);

    heap_end = heap_start + CHUNKSIZE;
    // if ((uint64_t)heap_start & (ALIGNMENT - 1))
    //     prologue =
    //         (void *)(((uint64_t)heap_start & ~(ALIGNMENT - 1)) + ALIGNMENT);
    // else
    //     prologue = heap_start;
    Ptr prologue = getPrologue();

    prologue.asHeader->data = 0;
    prologue.asHeader->allocated = true;
    (prologue.asFooter + 1)->data = 0;
    (prologue.asFooter + 1)->allocated = true;

    Ptr epilogue = getEpilogue();

    epilogue.asHeader->data = 0;
    epilogue.asHeader->allocated = true;
    (epilogue.asFooter + 1)->data = 0;
    (epilogue.asFooter + 1)->allocated = true;

    Ptr blankArea;
    blankArea.as_int = prologue.as_int + sizeof(Header) + sizeof(Footer);
    // todo

    return 0;
}

Ptr extendSbrk(size_t size)
{
    Ptr ret;
    assert(size == ALIGN(size));
    Ptr origBrk;
    origBrk.as_ptr = mem_sbrk(size);
    if (origBrk.as_int == -1)
    {
        ret.as_int = -1;
        return ret;
    }

    Ptr origEpilogue = getEpilogue();
    heap_end = heap_end + size;
    Ptr epilogue = getEpilogue();
    memcpy(epilogue.as_ptr, origEpilogue.as_ptr,
           sizeof(Header) + sizeof(Footer));

    Ptr blankArea = origEpilogue;
    blankArea.asHeader->data = size - sizeof(Header) - sizeof(Footer);
    blankArea.asHeader->allocated = false;

    Ptr blankAreaFooter;
    blankAreaFooter.as_int = epilogue.as_int - sizeof(Footer);
    blankAreaFooter.asFooter->data = size - sizeof(Header) - sizeof(Footer);
    blankAreaFooter.asFooter->allocated = false;

    return blankArea;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    Ptr currentBlock = getPrologue();
    Ptr epilogue = getEpilogue();
    while (currentBlock.as_ptr != epilogue.as_ptr)
    {
        if (!currentBlock.asHeader->allocated &&
            getHeaderBlocksize(currentBlock.asHeader) >= size)
        {
            currentBlock.asHeader->allocated = true;
            Footer *footer = getFooterFromHeader(currentBlock.asHeader);
            footer->allocated = true; // todo
            return currentBlock.as_ptr;
        }
        else
        {
            currentBlock.as_int += sizeof(Header) +
                                   getHeaderBlocksize(currentBlock.asHeader) +
                                   sizeof(Footer);
        }
    }
    int newsize = ALIGN(size + sizeof(Header) + sizeof(Footer));
    Ptr newBlockStart = extendSbrk(newsize);
    if (newBlockStart.as_int == -1)
        return NULL;
    else
    {
        newBlockStart.asHeader->allocated = true; // todo
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

size_t mm_check()
{
    return 0;
}