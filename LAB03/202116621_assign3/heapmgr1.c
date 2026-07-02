/*--------------------------------------------------------------------*/
/* heap manager (doubly linked free list)                             */
/* Author: Minsoo Kim                                                 */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "chunk.h"

#define FALSE 0
#define TRUE  1

#define NO_COALESCE 0
#define COALESCE_RIGHT 1
#define COALESCE_LEFT  2
#define COALESCE_BOTH  3

enum {
    MEMALLOC_MIN = 1024,
};

/* g_free_head: point to first chunk in the free list (LIFO) */
static Chunk_T g_free_head = NULL;

/* g_heap_start, g_heap_end: start and end of the heap area */
static void *g_heap_start = NULL;
static void *g_heap_end   = NULL;

#ifndef NDEBUG
static int 
check_heap_validity(void)
{
    Chunk_T w;
    size_t total_heap_units = ((char *)g_heap_end - (char *)g_heap_start)/CHUNK_UNIT;
    size_t heap_units= 0;

    if (g_heap_start == NULL || g_heap_end == NULL) {
        fprintf(stderr, "Uninitialized heap\n");
        return FALSE;
    }

    if (g_heap_start == g_heap_end) {
        if (g_free_head == NULL) {
            return TRUE;
        }
        fprintf(stderr, "Inconsistent empty heap\n");
        return FALSE;
    }

    for (w = (Chunk_T)g_heap_start;
         w && w < (Chunk_T)g_heap_end;
         w = chunk_get_next_adjacent(w, g_heap_start, g_heap_end))
    {
        if (!chunk_is_valid(w, g_heap_start, g_heap_end)) {
            return FALSE;
        }
        heap_units += chunk_get_units(w) + 2;
    }
    if (w) {
        fprintf(stderr, "Heap overflow: %p\n", (void *)w);
        return FALSE;
    }
    if (heap_units != total_heap_units) {
        fprintf(stderr, "Heap size mismatch: %zu vs %zu\n", heap_units, total_heap_units);
        return FALSE;
    }

    for (w = g_free_head; w; w = chunk_get_next_free_chunk(w))
    {
        if (chunk_get_status(w) != CHUNK_FREE) {
            return FALSE;
        }

        Chunk_T prev = chunk_get_prev_free_chunk(w);
        Chunk_T next = chunk_get_next_free_chunk(w);
        if (prev && chunk_get_next_free_chunk(prev) != w) {
            fprintf(stderr, "Prev→next inconsistency at %p\n", (void *)w);
            return FALSE;
        }
        if (next && chunk_get_prev_free_chunk(next) != w) {
            fprintf(stderr, "Next→prev inconsistency at %p\n", (void *)w);
            return FALSE;
        }
    }

    return TRUE;
}
#endif

/*--------------------------------------------------------------*/
/* size_to_units:                                              */
/*   Returns number of units required for 'size' bytes.         */
/*--------------------------------------------------------------*/
static size_t 
size_to_units(size_t size)
{
    return (size + (CHUNK_UNIT - 1)) / CHUNK_UNIT;
}

/*--------------------------------------------------------------*/
/* get_chunk_from_data_ptr:                                     */
/*   Returns the header pointer corresponding to data 'ptr'.     */
/*--------------------------------------------------------------*/
static Chunk_T 
get_chunk_from_data_ptr(void *ptr)
{
    return (Chunk_T)((char *)ptr - CHUNK_UNIT);
}

/*--------------------------------------------------------------*/
/* init_my_heap:                                                */
/*   Initialize heap start/end pointers.                        */
/*--------------------------------------------------------------*/
static void 
init_my_heap(void)
{
    g_heap_start = sbrk(0);
    g_heap_end   = g_heap_start;

    if (g_heap_start == (void *)-1) {
        perror("sbrk");
        exit(EXIT_FAILURE);
    }
}

/*--------------------------------------------------------------*/
/* insert_chunk:                                                */
/*   Insert chunk 'c' at head of free list (LIFO).              */
/*--------------------------------------------------------------*/
static void 
insert_chunk(Chunk_T c)
{
    assert(c >= (Chunk_T)g_heap_start && c < (Chunk_T)g_heap_end);
    assert(chunk_get_status(c) == CHUNK_FREE);
    assert(chunk_get_units(c) > 0);

    chunk_set_prev_free_chunk(c, NULL);
    chunk_set_next_free_chunk(c, g_free_head);

    if (g_free_head != NULL) {
        chunk_set_prev_free_chunk(g_free_head, c);
    }

    g_free_head = c;
}

/*--------------------------------------------------------------*/
/* merge_chunk:                                                 */
/*   Merge two adjacent free chunks c1 and c2; return merged.   */
/*--------------------------------------------------------------*/
static Chunk_T 
merge_chunk(Chunk_T c1, Chunk_T c2)
{
    assert(c1 < c2);
    assert(chunk_get_status(c1) == CHUNK_FREE);
    assert(chunk_get_status(c2) == CHUNK_FREE);
    assert(chunk_get_next_adjacent(c1, g_heap_start, g_heap_end) == c2);

    size_t units = chunk_get_units(c1) + chunk_get_units(c2) + 2;
    chunk_set_units(c1, units);
    return c1;
}

/*--------------------------------------------------------------*/
/* merge_chunks:                                                */
/*   Merge three adjacent free chunks c1, c2, c3; return merged.*/
/*--------------------------------------------------------------*/
static Chunk_T 
merge_chunks(Chunk_T c1, Chunk_T c2, Chunk_T c3)
{
    assert(c1 < c2 && c2 < c3);
    assert(chunk_get_next_adjacent(c1, g_heap_start, g_heap_end) == c2);
    assert(chunk_get_next_adjacent(c2, g_heap_start, g_heap_end) == c3);
    assert(chunk_get_status(c1) == CHUNK_FREE);
    assert(chunk_get_status(c2) == CHUNK_FREE);
    assert(chunk_get_status(c3) == CHUNK_FREE);

    size_t units = chunk_get_units(c1) + chunk_get_units(c2)
                   + chunk_get_units(c3) + 4;
    chunk_set_units(c1, units);

    return c1;
}

/*--------------------------------------------------------------*/
/* split_chunk:                                                 */
/*   Split free chunk 'c' into [c | alloc]                      */
/*   insert c into the free list                                */
/*   return the allocated chunk (with size units, IN USE).      */
/*--------------------------------------------------------------*/
static Chunk_T 
split_chunk(Chunk_T c, size_t units)
{
    size_t all_units = chunk_get_units(c);
    
    assert(c >= (Chunk_T)g_heap_start && c < (Chunk_T)g_heap_end);
    assert(all_units >= units + 3);
    assert(chunk_get_status(c) == CHUNK_FREE);

    chunk_set_units(c, all_units - units - 2);
    chunk_set_status(c, CHUNK_FREE);
    insert_chunk(c);

    Chunk_T alloc = chunk_get_next_adjacent(c, g_heap_start, g_heap_end);
    chunk_set_units(alloc, units);
    chunk_set_status(alloc, CHUNK_IN_USE);

    return alloc;
}

/*--------------------------------------------------------------------*/
/* remove_chunk_from_list:                                            */         
/*    Remove 'c' from the free chunk list.                            */
/*    get prev, next free chunk of c to maintain list                 */
/*--------------------------------------------------------------------*/
static void
remove_chunk_from_list(Chunk_T c)
{
    assert(chunk_get_status(c) == CHUNK_FREE);
    assert(c >= (Chunk_T)g_heap_start && c < (Chunk_T)g_heap_end);
    assert(chunk_get_units(c) > 0);

    Chunk_T prev = chunk_get_prev_free_chunk(c);
    Chunk_T next = chunk_get_next_free_chunk(c);

    if (prev)
        chunk_set_next_free_chunk(prev, next);
    else
        g_free_head = next;
    
    if (next)
        chunk_set_prev_free_chunk(next, prev);
    
    chunk_set_next_free_chunk(c, NULL);
    chunk_set_prev_free_chunk(c, NULL);
}

/*--------------------------------------------------------------*/
/* allocate_more_memory:                                        */
/*   Expand heap by required units or 1024 units                */
/*   merge with previous chunk (if free)                        */
/*   return new allocated free chunk. (not inserted in list)    */
/*--------------------------------------------------------------*/
static Chunk_T 
allocate_more_memory(size_t units)
{
    if (units < MEMALLOC_MIN)
        units = MEMALLOC_MIN;

    Chunk_T c = (Chunk_T)sbrk((units + 2) * CHUNK_UNIT);
    if (c == (Chunk_T)-1)
        return NULL;
    
    g_heap_end = sbrk(0);
    chunk_set_units(c, units);
    chunk_set_status(c, CHUNK_FREE);
    
    Chunk_T prev_chunk = chunk_get_prev_adjacent(c, g_heap_start, g_heap_end);
    if (prev_chunk && chunk_get_status(prev_chunk) == CHUNK_FREE) {
        remove_chunk_from_list(prev_chunk);
        c = merge_chunk(prev_chunk, c);
        return c;
    }
    
    return c;
}

/*--------------------------------------------------------------*/
/* heapmgr_malloc:                                              */
/*   Allocate 'size' bytes; substitute for malloc().            */
/*--------------------------------------------------------------*/
void 
*heapmgr_malloc(size_t size)
{
    static int is_init = FALSE;
    size_t units;
    Chunk_T c;

    if (size == 0)
        return NULL;
    
    if (is_init == FALSE) {
        init_my_heap();
        is_init = TRUE;
    }
    
    assert(check_heap_validity());

    units = size_to_units(size);

    for (c = g_free_head; 
         c != NULL; 
         c = chunk_get_next_free_chunk(c)) {
        
            // found large enough chunk to allocate
        if (chunk_get_units(c) >= units) {
            remove_chunk_from_list(c);
                // split if large enough (insert front part back to free list)
            if (chunk_get_units(c) >= units + 3) 
                c = split_chunk(c, units);
            chunk_set_status(c, CHUNK_IN_USE);
            assert(check_heap_validity());
            return (void *)((char *)c + CHUNK_UNIT);
        }
    }

    /* No large enough free chunk found; allocate more memory */
    if (c == NULL) {
        c = allocate_more_memory(units);
        // if allocation failed, return NULL
        if (c == NULL) {
            assert(check_heap_validity());
            return NULL;
        }
    }

    assert(chunk_get_units(c) >= units);
    if (chunk_get_units(c) >= units + 3)
        c = split_chunk(c, units);
    chunk_set_status(c, CHUNK_IN_USE);
    assert(check_heap_validity());
    return (char *)c + CHUNK_UNIT;
}

/*--------------------------------------------------------------*/
/* heapmgr_free:                                                */
/*   Free block at 'ptr'; substitute for free().                */
/*--------------------------------------------------------------*/
void 
heapmgr_free(void *m)
{
    Chunk_T c;

    if (m == NULL)
        return;

    assert(check_heap_validity());

    c = get_chunk_from_data_ptr(m);
    assert(chunk_get_status(c) == CHUNK_IN_USE);
    assert(c >= (Chunk_T)g_heap_start && c < (Chunk_T)g_heap_end);
    assert(chunk_get_units(c) > 0);

    chunk_set_status(c, CHUNK_FREE);

    /* Coalesce with neighbors if free */
    Chunk_T next_chunk = chunk_get_next_adjacent(c, g_heap_start, g_heap_end);
    Chunk_T prev_chunk = chunk_get_prev_adjacent(c, g_heap_start, g_heap_end);
    
    unsigned int flag = 0;
    flag |= (next_chunk && chunk_get_status(next_chunk) == CHUNK_FREE);  /* bit0 */
    flag |= (prev_chunk && chunk_get_status(prev_chunk) == CHUNK_FREE) << 1;  /* bit1 */
    
    switch (flag) {
        case COALESCE_BOTH:  /* 0b11 */
            remove_chunk_from_list(prev_chunk);
            remove_chunk_from_list(next_chunk);
            c = merge_chunks(prev_chunk, c, next_chunk);
            break;
        case COALESCE_LEFT:  /* 0b10 */
            remove_chunk_from_list(prev_chunk);
            c = merge_chunk(prev_chunk, c);
            break;
        case COALESCE_RIGHT:  /* 0b01 */
            remove_chunk_from_list(next_chunk);
            c = merge_chunk(c, next_chunk);
            break;
        case NO_COALESCE:  /* 0b00 */
            break;
        default:
            fprintf(stderr, "Invalid coalescing flag: %u\n", flag);
            assert(0);
    }
    insert_chunk(c);
    assert(check_heap_validity());
}
