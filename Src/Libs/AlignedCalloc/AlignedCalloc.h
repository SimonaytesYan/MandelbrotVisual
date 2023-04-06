#ifndef SYM_ALIGNED_CALLOC
#define SYM_ALIGNED_CALLOC

//!
//!@return aligned address
//!
void* AlignedCalloc(void** addres, const size_t size, const size_t alignment);
void AlignedFree(void* addres);

#endif
