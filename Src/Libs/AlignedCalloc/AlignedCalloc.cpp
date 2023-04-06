#include <stdlib.h>
#include <stdio.h>

#include "AlignedCalloc.h"

//#define DEBUG

void* AlignedCalloc(void** addres, const size_t size, const size_t alignment)
{
    *addres = calloc(1, size + alignment); 
    size_t diff_to_align = alignment - (size_t)(*addres) % alignment;

    #ifdef DEBUG
        printf("diff_to_align = %zu\n", diff_to_align);
        printf("aligment      = %zu\n", alignment);
        printf("address       = %p\n",  *addres);
        printf("address_st    = %p\n\n",  (size_t)*addres);
    #endif

    if (diff_to_align == alignment)
        return *addres;

    void* aligned_addres = (char*)*addres + diff_to_align;
    return aligned_addres;
}

void AlignedFree(void* addres)
{
    free(addres);
}