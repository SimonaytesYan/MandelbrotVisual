#include <stdlib.h>
#include <stdio.h>

#include "AlignedCalloc.h"

//#define DEBUG

//! @return alignmented addresss 
void* AlignedCalloc(void** address, const size_t size, const size_t alignment)
{
    *address = calloc(1, size + alignment); 
    size_t diff_to_align = alignment - (size_t)(*address) % alignment;

    #ifdef DEBUG
        printf("diff_to_align = %zu\n", diff_to_align);
        printf("alignment     = %zu\n", alignment);
        printf("address       = %p\n",  *address);
        printf("address_st    = %p\n\n",  (size_t)*address);
    #endif

    if (diff_to_align == alignment)
        return *address;

    void* aligned_address = (char*)*address + diff_to_align;
    return aligned_address;
}

void AlignedFree(void* address)
{
    free(address);
}