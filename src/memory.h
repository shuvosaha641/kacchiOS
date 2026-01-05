#ifndef KACCHI_MEMORY_H
#define KACCHI_MEMORY_H

#include "types.h"

/*
 * KacchiOS memory layout configuration.
 * You can tweak these if your RAM size changes.
 */
#define KACCHI_STACK_BYTES 4096
#define KACCHI_HEAP_BYTES  8192

/* Keep the original macro names for compatibility */
#define STACK_SIZE KACCHI_STACK_BYTES
#define HEAP_SIZE  KACCHI_HEAP_BYTES

/*
 * Backing storage for the "kernel stack" and "kernel heap".
 * Exposed via macros stack / heap to preserve external usage,
 * but the actual symbols have different names.
 */
extern uint8_t g_stack_store[STACK_SIZE];
extern uint8_t g_heap_store[HEAP_SIZE];

#define stack g_stack_store
#define heap  g_heap_store

/*
 * Heap segment descriptor.
 * Internally the allocator treats the heap as a linked list
 * of these segments.
 */
typedef struct HeapSegment {
    size_t        length;   /* size of the usable payload in bytes */
    int           is_free;  /* non-zero if this segment is free */
    struct HeapSegment *link; /* next segment in the heap list */
} HeapSegment;

/* For compatibility with any existing code using MemBlock */
typedef HeapSegment MemBlock;

/* ----------------------------------------------------------------------------
 * Public interface
 * ----------------------------------------------------------------------------
 */

/* Initialize stack/heap data structures – call once during kernel startup. */
void memory_init(void);

/* Simple linear stack allocator / deallocator (LIFO style). */
void *stack_alloc(size_t size);
void  stack_free(size_t size);

/* Best-fit heap allocator with splitting and coalescing. */
void *heap_alloc(size_t size);
void  heap_free(void *ptr);

/* Optional diagnostic routine to exercise the allocator. */
void stress_test_memory(void);

#endif /* KACCHI_MEMORY_H */
