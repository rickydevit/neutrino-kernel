#pragma once

#include <size_t.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __kernel
    #define Prefix(func)		k ## func
#else
    #define Prefix(func)        func
#endif

#define ALLOC_MARKER_MAGIC	0xc001c0de
#define ALLOC_MARKER_DEAD	0xdeaddead

#define ALLOC_PAGE_SIZE 0x1000			// The size of an individual page. Set up in liballoc_init.
#define ALLOC_PAGE_COUNT 16			    // The number of pages to request per chunk. Set up in liballoc_init.

typedef struct __alloc_major AllocMajor;
typedef struct __alloc_minor AllocMinor;

// *A structure found at the top of all system allocated 
// *memory blocks. It details the usage of the memory block.
struct __alloc_major {
	uint32_t pages;			// The number of pages in the block.
	uint32_t size;			// The number of pages in the block.
	uint32_t usage;			// The number of bytes used in the block.
	AllocMajor* prev;		// Linked list information.
	AllocMajor* next;		// Linked list information.
	AllocMinor* first;		// A pointer to the first allocated memory in the block.	
};

// *This is a structure found at the beginning of all
// *sections in a major block which were allocated by a
// *malloc, calloc, realloc call.
struct __alloc_minor {
	uint32_t magic;			// A magic number to idenfity correctness.
	uint32_t size; 			// The size of the memory allocated. Could be 1 byte or more.
	uint32_t req_size;		// The size of memory requested.
	AllocMinor* prev;		// Linked list information.
	AllocMinor* next;		// Linked list information.
	AllocMajor* block;		// The owning block. A pointer to the major structure.
};

extern int liballoc_lock();
extern int liballoc_unlock();
extern void* liballoc_alloc(size_t);
extern int liballoc_free(void*,size_t);
extern bool liballoc_try_lock();
    
extern void* Prefix(malloc)(size_t);
extern void* Prefix(realloc)(void*, size_t);
extern void* Prefix(calloc)(size_t, size_t);
extern void Prefix(free)(void*);

static inline void* lmalloc(size_t s) {
#ifdef __kernel
    return kmalloc(s);
#else
    return malloc(s);
#endif
}

static inline void lfree(void* p) {
#ifdef __kernel
    kfree(p);
#else
    free(p);
#endif
}

static inline void* lrealloc(void* p, size_t s) {
#ifdef __kernel
    return krealloc(p, s);
#else
    return realloc(p, s);
#endif
}

static inline void* lcalloc(size_t s, size_t n) {
#ifdef __kernel
    return kcalloc(s, n);
#else
    return calloc(s, n);
#endif
}
