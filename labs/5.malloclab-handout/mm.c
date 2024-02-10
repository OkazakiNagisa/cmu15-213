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

Header *HeaderGetNextHeader(Header *header)
{
    Ptr ret;
    ret.as_ptr = header;
    uint32_t blockSize = HeaderGetBlocksize(header);
    ret.as_int += sizeof(Header) + blockSize + sizeof(Footer);
    return ret.asHeader;
}

Header *HeaderGetPreviousHeader(Header *header)
{
    Ptr ret;
    ret.as_ptr = header;
    Footer *prevFooter = ret.asHeader - 1;
    return FooterGetHeader(prevFooter);
}

void MemSetBlockData(Header *header, bool allocated, uint32_t newSize)
{
    Ptr footer;
    footer.as_ptr = header;
    assert(newSize == ALIGN(newSize));
    header->data = newSize;
    header->allocated = allocated;
    footer.asFooter = HeaderGetFooter(header);
    footer.asFooter->data = header->data;
}

int32_t MemSetupLayout()
{
    // sbrk
    mem.RawHeapStart.as_ptr = mem_sbrk(CHUNKSIZE);
    if (mem.RawHeapStart.as_int == -1)
        return -1;
    mem.HeapStart.as_int = ALIGN(mem.RawHeapStart.as_int);
    mem.HeapEnd.as_int = mem.HeapStart.as_int + CHUNKSIZE;

    // layout
    Ptr prologue = MemGetPrologue();
    MemSetBlockData(prologue.asHeader, false, 0);

    Ptr epilogue = MemGetEpilogue();
    MemSetBlockData(epilogue.asHeader, false, 0);

    Ptr blankArea;
    blankArea.as_int = prologue.as_int + sizeof(Header) + sizeof(Footer);
    size_t blankAreaSize =
        epilogue.as_int - prologue.as_int - sizeof(Header) - sizeof(Footer);

    MemSetBlockData(blankArea.asHeader, false, blankAreaSize);

    return 0;
}

int32_t MemExtendLayout(size_t desiredSize)
{
    assert(desiredSize == ALIGN(desiredSize));

    // sbrk
    Ptr origBrk;
    origBrk.as_ptr = mem_sbrk(desiredSize + sizeof(Header) + sizeof(Footer));
    if (origBrk.as_int == -1)
        return -1;

    // layout
    Ptr origEpilogue = MemGetEpilogue();
    mem.HeapEnd.as_int += desiredSize + sizeof(Header) + sizeof(Footer);
    Ptr newEpilogue = MemGetEpilogue();
    memcpy(newEpilogue.as_ptr, origEpilogue.as_ptr,
           sizeof(Header) + sizeof(Footer));

    Ptr blankAreaHeader = origEpilogue;
    MemSetBlockData(blankAreaHeader.asHeader, false, desiredSize);

    return blankAreaHeader.as_int;
}

Header *MemSplitBlock(Header *blockHeader, size_t firstBlockSize)
{
    uint32_t origSize = HeaderGetBlocksize(blockHeader);
    assert(firstBlockSize == ALIGN(firstBlockSize));
    assert(firstBlockSize + sizeof(Header) + sizeof(Footer) < origSize);
    MemSetBlockData(blockHeader, blockHeader->allocated, firstBlockSize);

    uint32_t nextBlockSize =
        origSize - firstBlockSize - sizeof(Header) - sizeof(Footer);
    Header *nextHeader = HeaderGetNextHeader(blockHeader);
    MemSetBlockData(nextHeader, false, nextBlockSize);

    return nextHeader;
}

void MemMergeFreeBlocks(Header *blockHeader, bool mergePrevious)
{
    Header *first =
        mergePrevious ? HeaderGetPreviousHeader(blockHeader) : blockHeader;
    Header *second =
        mergePrevious ? blockHeader : HeaderGetNextHeader(blockHeader);
    assert(!first->allocated && !second->allocated);
    MemSetBlockData(first, false,
                    HeaderGetBlocksize(first) + HeaderGetBlocksize(second));
}

void MemFreeBlock(Header *blockHeader)
{
    assert(blockHeader->allocated);
    MemSetBlockData(blockHeader, false, HeaderGetBlocksize(blockHeader));
    Header *prevHeader = HeaderGetPreviousHeader(blockHeader);
    Header *nextHeader = HeaderGetNextHeader(blockHeader);
    if (!nextHeader->allocated)
        MemMergeFreeBlocks(blockHeader, false);
    if (!prevHeader->allocated)
        MemMergeFreeBlocks(prevHeader, true);
}

// return true = succ
bool MemTryEnlargeBlock(Header *blockHeader, size_t increment)
{
    assert(increment == ALIGN(increment));
    uint32_t origSize = HeaderGetBlocksize(blockHeader);
    uint32_t desiredSize = origSize + increment;
    Header *nextHeader = HeaderGetNextHeader(blockHeader);
    uint32_t nextBlockSize = HeaderGetBlocksize(nextHeader);

    if (nextBlockSize < increment)
        return false;

}

bool MemTryShrinkBlock(Header *blockHeader, size_t decrement)
{
    assert(decrement == ALIGN(decrement));
    uint32_t origSize = HeaderGetBlocksize(blockHeader);
    assert(decrement < origSize);
    uint32_t desiredSize = origSize - decrement;

    if (desiredSize + sizeof(Header) + sizeof(Footer) >= origSize)
        return false;

    MemSplitBlock(blockHeader, desiredSize);
    return true;
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    return MemSetupLayout();
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size = ALIGN(size);
    Ptr currentBlock = MemGetPrologue();
    Ptr epilogue = MemGetEpilogue();
    while (currentBlock.as_ptr != epilogue.as_ptr)
    {
        if (!currentBlock.asHeader->allocated &&
            HeaderGetBlocksize(currentBlock.asHeader) >= size)
        {
            if (size + sizeof(Header) + sizeof(Footer) <
                HeaderGetBlocksize(currentBlock.asHeader))
            {
                MemSplitBlock(currentBlock.asHeader, size);
            }
            currentBlock.asHeader++;
            return currentBlock.as_ptr;
        }
        else
        {
            currentBlock.asHeader = HeaderGetNextHeader(currentBlock.asHeader);
        }
    }
    Ptr newBlockStart;
    newBlockStart.as_int = MemExtendLayout(ALIGN(size));
    if (newBlockStart.as_int == -1)
        return NULL;
    else
    {
        MemSetBlockData(newBlockStart.asHeader, true,
                        HeaderGetBlocksize(newBlockStart.asHeader));
        currentBlock.asHeader++;
        return newBlockStart.as_ptr;
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    Header *header = ptr;
    header--;
    MemFreeBlock(header);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    Header *header = ptr;
    header--;
    uint32_t blockSize = HeaderGetBlocksize(header);

    if (size <= blockSize)
    {
    }
    // void *oldptr = ptr;
    // void *newptr;
    // size_t copySize;

    // newptr = mm_malloc(size);
    // if (newptr == NULL)
    //     return NULL;
    // copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    // if (size < copySize)
    //     copySize = size;
    // memcpy(newptr, oldptr, copySize);
    // mm_free(oldptr);
    // return newptr;
}

size_t mm_check()
{
    return 0;
}