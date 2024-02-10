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
#include <cstddef>
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
        bool unused1   : 1;
        bool unused2   : 1;
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

typedef struct
{
    Ptr RawHeapStart;
    Ptr HeapStart; // aligned
    Ptr HeapEnd;   // aligned
} Mem;

Mem mem;

Footer *HeaderGetFooter(Header *header);

Ptr MemGetPrologue()
{
    Ptr ret;
    ret.as_ptr = mem.HeapStart.as_ptr;
    return ret;
}

Ptr MemGetEpilogue()
{
    Ptr ret;
    ret.as_ptr = mem.HeapEnd.as_ptr;
    ret.asHeader -= 2;
    return ret;
}

void MemSetBlockData(Header *header, bool allocated, bool overrideSize,
                     uint32_t newSize)
{
    Ptr footer;
    footer.as_ptr = header;
    if (!overrideSize)
    {
        footer.asFooter = HeaderGetFooter(header);
    }
    else
    {
        assert(newSize == ALIGN(newSize));
        footer.as_int += newSize;
    }
    header->data = newSize;
    header->allocated = allocated;
    footer.asFooter->data = newSize;
    footer.asFooter->allocated = allocated;
}

void MemSetupLayout()
{
    Ptr prologue = MemGetPrologue();
    MemSetBlockData(prologue.asHeader, false, true, 0);

    Ptr epilogue = MemGetEpilogue();
    MemSetBlockData(epilogue.asHeader, false, true, 0);

    Ptr blankArea;
    blankArea.as_int = prologue.as_int + sizeof(Header) + sizeof(Footer);
    size_t blankAreaSize =
        epilogue.as_int - prologue.as_int - sizeof(Header) - sizeof(Footer);

    Ptr blankAreaHeader = blankArea;
    blankAreaHeader.asHeader->data = blankAreaSize;
    blankAreaHeader.asHeader->allocated = false;

    Ptr blankAreaFooter = blankArea;
    blankAreaFooter.as_int += blankAreaSize - sizeof(Footer);
    blankAreaFooter.asFooter->data = blankAreaSize;
    blankAreaFooter.asFooter->allocated = false;
}

Ptr MemExtendLayout(size_t size)
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

    Ptr origEpilogue = MemGetEpilogue();
    mem.HeapEnd.as_int += size;
    Ptr newEpilogue = MemGetEpilogue();
    memcpy(newEpilogue.as_ptr, origEpilogue.as_ptr,
           sizeof(Header) + sizeof(Footer));

    Ptr blankAreaHeader = origEpilogue;
    blankAreaHeader.asHeader->data = size - sizeof(Header) - sizeof(Footer);
    blankAreaHeader.asHeader->allocated = false;

    Ptr blankAreaFooter;
    blankAreaFooter.as_int = newEpilogue.as_int - sizeof(Footer);
    blankAreaFooter.asFooter->data = size - sizeof(Header) - sizeof(Footer);
    blankAreaFooter.asFooter->allocated = false;

    return blankAreaHeader;
}

uint32_t HeaderGetBlocksize(Header *header)
{
    return header->data & ~0b111;
}

Footer *HeaderGetFooter(Header *header)
{
    Ptr ret;
    ret.as_ptr = header;
    uint32_t blockSize = HeaderGetBlocksize(header);
    ret.as_int += sizeof(Header) + blockSize;
    return ret.asFooter;
}

Header *FooterGetHeader(Footer *footer)
{
    Ptr ret;
    ret.as_ptr = footer;
    uint32_t blockSize = HeaderGetBlocksize(footer);
    ret.as_int -= sizeof(Footer) + blockSize;
    return ret.asHeader;
}

void MemSplitBlock(Ptr block, size_t size)
{
    uint32_t blockSize = HeaderGetBlocksize(block.asHeader);
    assert(size + sizeof(Header) + sizeof(Footer) < blockSize);
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    mem.RawHeapStart.as_ptr = mem_sbrk(CHUNKSIZE);
    if (mem.RawHeapStart.as_int == -1)
        return -1;
    mem.HeapStart.as_int = ALIGN(mem.RawHeapStart.as_int);
    mem.HeapEnd.as_int = mem.HeapStart.as_int + CHUNKSIZE;

    MemSetupLayout();

    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    Ptr currentBlock = MemGetPrologue();
    Ptr epilogue = MemGetEpilogue();
    while (currentBlock.as_ptr != epilogue.as_ptr)
    {
        if (!currentBlock.asHeader->allocated &&
            HeaderGetBlocksize(currentBlock.asHeader) >= size)
        {
            currentBlock.asHeader->allocated = true;
            Footer *footer = HeaderGetFooter(currentBlock.asHeader);
            footer->allocated = true; // todo
            return currentBlock.as_ptr;
        }
        else
        {
            currentBlock.as_int += sizeof(Header) +
                                   HeaderGetBlocksize(currentBlock.asHeader) +
                                   sizeof(Footer);
        }
    }
    int newsize = ALIGN(size + sizeof(Header) + sizeof(Footer));
    Ptr newBlockStart = MemExtendLayout(newsize);
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