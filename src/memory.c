#include "memory.h"
#include "serial.h"

/*
 * Backing arrays for stack and heap.
 * Exposed as 'stack' / 'heap' via macros in memory.h.
 */
uint8_t g_stack_store[STACK_SIZE];
uint8_t g_heap_store[HEAP_SIZE];

/* Current top of the stack region (offset into g_stack_store) */
static size_t stack_marker = 0;

/* Head of the heap segment linked list */
static HeapSegment *heap_head_node = NULL;

/* --------------------------------------------------------------------------
 * Internal helpers
 * -------------------------------------------------------------------------- */

/* Align a value up to the next 4-byte boundary */
static size_t align_to_4(size_t value)
{
    const size_t mask = 4u - 1u;
    return (value + mask) & ~mask;
}

/* Merge consecutive free segments in the heap list */
static void merge_adjacent_free_segments(void)
{
    HeapSegment *node = heap_head_node;

    while (node && node->link)
    {
        HeapSegment *next = node->link;

        if (node->is_free && next->is_free)
        {
            /* Fold 'next' into 'node' */
            node->length += sizeof(HeapSegment) + next->length;
            node->link = next->link;
            /* do NOT advance node; there might be another free segment */
        }
        else
        {
            node = next;
        }
    }
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

void memory_init(void)
{
    /*
     * Initialize the heap as a single free segment spanning all of g_heap_store.
     */
    heap_head_node = (HeapSegment *)g_heap_store;
    heap_head_node->length = HEAP_SIZE - sizeof(HeapSegment);
    heap_head_node->is_free = 1;
    heap_head_node->link = NULL;

    /*
     * Reset stack "top" offset.
     */
    stack_marker = 0;
}

/* ---------------- Stack allocator (bump + manual pop) ------------------ */

void *stack_alloc(size_t size)
{
    if (size == 0)
    {
        return NULL;
    }

    if (stack_marker + size > STACK_SIZE)
    {
        /* Out of stack space */
        return NULL;
    }

    void *addr = &g_stack_store[stack_marker];
    stack_marker += size;
    return addr;
}

void stack_free(size_t size)
{
    /*
     * Linear stack model: we only support freeing from the top by size.
     * If caller asks to free more than we have, just reset.
     */
    if (size >= stack_marker)
    {
        stack_marker = 0;
    }
    else
    {
        stack_marker -= size;
    }
}

/* ---------------- Heap allocator (best-fit + coalescing) --------------- */

void *heap_alloc(size_t size)
{
    if (size == 0)
    {
        return NULL;
    }

    /* 1. Align requested size to 4 bytes */
    size = align_to_4(size);

    HeapSegment *best = NULL;
    HeapSegment *scan = heap_head_node;

    /*
     * 2. Best-fit search: find the smallest free segment that can satisfy size.
     */
    while (scan)
    {
        if (scan->is_free && scan->length >= size)
        {
            if (best == NULL || scan->length < best->length)
            {
                best = scan;
            }
            if (scan->length == size)
            {
                /* Perfect fit, we can stop early */
                break;
            }
        }
        scan = scan->link;
    }

    if (best == NULL)
    {
        /* No suitable free segment available */
        return NULL;
    }

    /*
     * 3. Decide if we should split the segment.
     * Only split when leftover is large enough for a header + some payload.
     */
    const size_t min_payload = 4;
    const size_t min_remainder = sizeof(HeapSegment) + min_payload;

    if (best->length >= size + min_remainder)
    {
        uint8_t *base = (uint8_t *)best;
        uint8_t *new_seg_addr = base + sizeof(HeapSegment) + size;

        HeapSegment *split = (HeapSegment *)new_seg_addr;
        split->length = best->length - size - sizeof(HeapSegment);
        split->is_free = 1;
        split->link = best->link;

        best->length = size;
        best->link = split;
    }

    best->is_free = 0;

    /* Return pointer to usable payload right after the header */
    return (uint8_t *)best + sizeof(HeapSegment);
}

void heap_free(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    /*
     * Recover header from payload pointer.
     */
    HeapSegment *seg = (HeapSegment *)((uint8_t *)ptr - sizeof(HeapSegment));
    seg->is_free = 1;

    /*
     * Perform full coalescing of adjacent free segments.
     */
    merge_adjacent_free_segments();
}

/* -------------------- Stress / validation routine ---------------------- */

void stress_test_memory(void)
{
    serial_puts("\n=== KacchiOS Memory Self-Test ===\n");

    /* Phase 1: Stack allocation / deallocation */
    serial_puts("Phase 1: testing stack allocator...\n");
    void *s1 = stack_alloc(100);
    if (s1 != NULL)
    {
        serial_puts("  -> 100 bytes allocated on stack.\n");
        stack_free(100);
        serial_puts("  -> 100 bytes released from stack. OK.\n");
    }
    else
    {
        serial_puts("  -> Stack allocation failed unexpectedly.\n");
    }

    /* Phase 2: Heap fragmentation and coalescing */
    serial_puts("Phase 2: heap fragmentation / merge test...\n");

    void *p1 = heap_alloc(512);
    void *p2 = heap_alloc(512);
    void *p3 = heap_alloc(512);

    if (!p1 || !p2 || !p3)
    {
        serial_puts("  -> Unable to allocate 3 × 512-byte heap blocks.\n");
        return;
    }
    serial_puts("  -> Successfully allocated three 512-byte heap blocks.\n");

    serial_puts("  -> Freeing all three blocks to trigger coalescing...\n");
    heap_free(p1);
    heap_free(p2);
    heap_free(p3);

    /* Phase 3: Check if coalescing created a larger free region */
    void *big = heap_alloc(1024);
    if (big)
    {
        serial_puts("  -> SUCCESS: 1024-byte allocation succeeded after merge.\n");
        heap_free(big);
    }
    else
    {
        serial_puts("  -> FAILURE: Heap still fragmented; 1024-byte block not available.\n");
    }

    serial_puts("=== Memory Self-Test Complete ===\n\n");
}
