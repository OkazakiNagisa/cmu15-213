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
#include <stddef.h>
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
#define ALIGN2CHUNK(size)                                                      \
    (((size_t)(size) + (CHUNKSIZE - 1)) & ~(CHUNKSIZE - 1))
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

typedef union
{
    struct
    {
        bool allocated : 1;
        bool unused1   : 1;
        bool unused2   : 1;
    };
    size_t data;
    uint64_t for_padding;
} Header, Footer;

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

int32_t mm_check();

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

size_t HeaderGetBlocksize(Header *header)
{
    return header->data & ~0b111;
}

Footer *HeaderGetFooter(Header *header)
{
    Ptr ret;
    ret.as_ptr = header;
    size_t blockSize = HeaderGetBlocksize(header);
    ret.as_int += sizeof(Header) + blockSize;
    return ret.asFooter;
}

Header *FooterGetHeader(Footer *footer)
{
    Ptr ret;
    ret.as_ptr = footer;
    size_t blockSize = HeaderGetBlocksize(footer);
    ret.as_int -= sizeof(Footer) + blockSize;
    return ret.asHeader;
}

Header *HeaderGetNextHeader(Header *header)
{
    Ptr ret;
    ret.as_ptr = header;
    size_t blockSize = HeaderGetBlocksize(header);
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

void MemSetBlockData(Header *header, bool allocated, size_t size)
{
    assert(size == ALIGN(size));
    header->data = size;
    header->allocated = allocated;
    Footer *footer = HeaderGetFooter(header);
    footer->data = header->data;
}

// return true = succ
bool MemSetupLayout()
{
    // sbrk
    mem.RawHeapStart.as_ptr = mem_sbrk(CHUNKSIZE);
    if (mem.RawHeapStart.as_int == -1)
        return false;
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

    assert(!mm_check());
    return true;
}

Header *MemExtendLayout(size_t leastSize)
{
    size_t extendedSize =
        ALIGN2CHUNK(leastSize + sizeof(Header) + sizeof(Footer));
    assert(extendedSize == ALIGN(extendedSize));

    // sbrk
    Ptr origBrk;
    origBrk.as_ptr = mem_sbrk(extendedSize);
    if (origBrk.as_int == -1)
        return NULL;

    // layout
    Ptr origEpilogue = MemGetEpilogue();
    mem.HeapEnd.as_int += extendedSize;
    Ptr newEpilogue = MemGetEpilogue();
    memcpy(newEpilogue.as_ptr, origEpilogue.as_ptr,
           sizeof(Header) + sizeof(Footer));

    Ptr blankAreaHeader = origEpilogue;
    MemSetBlockData(blankAreaHeader.asHeader, false,
                    extendedSize - sizeof(Header) - sizeof(Footer));

    assert(!mm_check());
    return blankAreaHeader.asHeader;
}

Header *MemSplitBlock(Header *blockHeader, size_t firstBlockSize)
{
    size_t origSize = HeaderGetBlocksize(blockHeader);
    assert(firstBlockSize == ALIGN(firstBlockSize));
    assert(firstBlockSize + sizeof(Header) + sizeof(Footer) < origSize);
    MemSetBlockData(blockHeader, blockHeader->allocated, firstBlockSize);

    size_t nextBlockSize =
        origSize - firstBlockSize - sizeof(Header) - sizeof(Footer);
    Header *nextHeader = HeaderGetNextHeader(blockHeader);
    MemSetBlockData(nextHeader, false, nextBlockSize);

    assert(!mm_check());
    return nextHeader;
}

void MemMergeBlocks(Header *blockHeader, bool mergePrevious,
                    bool mergedBlockAllocated)
{
    Header *first =
        mergePrevious ? HeaderGetPreviousHeader(blockHeader) : blockHeader;
    Header *second =
        mergePrevious ? blockHeader : HeaderGetNextHeader(blockHeader);
    assert(!second->allocated);
    MemSetBlockData(first, mergedBlockAllocated,
                    HeaderGetBlocksize(first) + HeaderGetBlocksize(second) +
                        sizeof(Header) + sizeof(Footer));
    assert(!mm_check());
}

void MemFreeBlock(Header *blockHeader)
{
    assert(blockHeader->allocated);
    MemSetBlockData(blockHeader, false, HeaderGetBlocksize(blockHeader));
    Header *prevHeader = HeaderGetPreviousHeader(blockHeader);
    Header *nextHeader = HeaderGetNextHeader(blockHeader);
    if (!nextHeader->allocated)
        MemMergeBlocks(blockHeader, false, false);
    if (!prevHeader->allocated)
        MemMergeBlocks(prevHeader, true, false);
}

// return true = succ
bool MemTryEnlargeBlock(Header *blockHeader, size_t increment)
{
    assert(increment == ALIGN(increment));
    size_t origSize = HeaderGetBlocksize(blockHeader);
    size_t desiredSize = origSize + increment;
    Header *nextHeader = HeaderGetNextHeader(blockHeader);
    size_t nextBlockSize = HeaderGetBlocksize(nextHeader);

    if (increment > nextBlockSize)
    {
        if (increment <= nextBlockSize + sizeof(Header) + sizeof(Footer))
        {
            MemMergeBlocks(blockHeader, false, blockHeader->allocated);
            return true;
        }
        else
            return false;
    }
    else
    {
        MemMergeBlocks(blockHeader, false, blockHeader->allocated);
        MemSplitBlock(blockHeader, desiredSize);
        return true;
    }
}

bool MemTryShrinkBlock(Header *blockHeader, size_t decrement)
{
    assert(decrement == ALIGN(decrement));
    size_t origSize = HeaderGetBlocksize(blockHeader);
    assert(decrement < origSize);
    size_t desiredSize = origSize - decrement;

    if (desiredSize + sizeof(Header) + sizeof(Footer) >= origSize)
        return false;

    MemSplitBlock(blockHeader, desiredSize);
    assert(!mm_check());
    return true;
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    return MemSetupLayout() ? 0 : -1;
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
            MemTryShrinkBlock(currentBlock.asHeader, size);
            MemSetBlockData(currentBlock.asHeader, true,
                            HeaderGetBlocksize(currentBlock.asHeader));
            currentBlock.asHeader++;
            return currentBlock.as_ptr;
        }
        else
        {
            currentBlock.asHeader = HeaderGetNextHeader(currentBlock.asHeader);
        }
    }
    Ptr newBlockStart;
    newBlockStart.asHeader = MemExtendLayout(size);
    if (!newBlockStart.as_ptr)
        return NULL;
    else
    {
        MemTryShrinkBlock(newBlockStart.asHeader, size);
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
    if (!ptr)
        return mm_malloc(size);
    if (!size)
    {
        mm_free(ptr);
        return NULL;
    }

    Header *header = ptr;
    header--;
    size_t blockSize = HeaderGetBlocksize(header);

    if (size <= blockSize)
    {
        MemTryShrinkBlock(header, blockSize - size);
        return ptr;
    }
    else
    {
        if (MemTryEnlargeBlock(header, size - blockSize))
        {
            return ptr;
        }
        else
        {
            void *newlyAllocated = mm_malloc(size);
            if (!newlyAllocated)
                return NULL;
            memcpy(newlyAllocated, ptr, blockSize);
            mm_free(ptr);
            return newlyAllocated;
        }
    }
}

int32_t mm_check()
{
    Ptr start = mem.HeapStart;
    Ptr end = mem.HeapEnd;

    Ptr currentBlock = start;
    while (currentBlock.as_ptr < end.as_ptr)
    {
        Header *currentBlockHeader = currentBlock.asHeader;
        size_t currentBlockSize = HeaderGetBlocksize(currentBlockHeader);
        Footer *currentBlockFooter = HeaderGetFooter(currentBlockHeader);

        if (currentBlockHeader->data != currentBlockFooter->data)
        {
            printf("!");
            return 1;
        }

        if (currentBlockHeader->data == 0 &&
            currentBlockHeader != MemGetPrologue().asHeader &&
            currentBlockHeader != MemGetEpilogue().asHeader)
        {
            printf("!");
            return 1;
        }

        if (HeaderGetNextHeader(currentBlockHeader) !=
            currentBlock.as_ptr + currentBlockSize + sizeof(Header) +
                sizeof(Footer))
        {
            printf("!");
            return 1;
        }
        currentBlock.as_ptr = HeaderGetNextHeader(currentBlockHeader);
    }

    if (currentBlock.as_ptr != end.as_ptr)
    {
        printf("!");
        return 1;
    }

    return 0;
}