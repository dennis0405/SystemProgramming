/*--------------------------------------------------------------------*/
/* Modified heap manager with segregated free lists (binning)         */
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
    NUM_BINS = 12
};

/* g_heap_start, g_heap_end: start and end of the heap area.         */
/* g_heap_end will move if you increase the heap                     */
static void *g_heap_start = NULL, *g_heap_end = NULL;

/* Segregated free lists (bins)                                      */
/* managed by using power of 2 (size of units)                       */
/* 1, 2, 3~4, 5~8, 9~16, ... , 513~1024, over 1024                   */
static Chunk_T g_free_bins[NUM_BINS] = { NULL };

/* function prototype                                                */
static int get_bin_index(size_t units);

#ifndef NDEBUG
/* check_heap_validity:                                              */
/* Validity check for entire data structures for chunks. Note that   */
/* this is basic sanity check, and passing this test does not        */
/* guarantee the integrity of your code.                             */
/* Returns 1 on success or 0 (zero) on failure.                      */
static int 
check_heap_validity(void)
{
    Chunk_T w;

    if (g_heap_start == NULL) {
        fprintf(stderr, "Uninitialized heap start\n");
        return FALSE;
    }

    if (g_heap_end == NULL) {
        fprintf(stderr, "Uninitialized heap end\n");
        return FALSE;
    }

    if (g_heap_start == g_heap_end) {
        for (int i = 0; i < NUM_BINS; i++) {
            if (g_free_bins[i]) {
                fprintf(stderr, "Inconsistent empty heap: bin %d not empty\n", i);
                return FALSE;
            }
        }
        return TRUE;
    }

    for (w = (Chunk_T)g_heap_start;
         w && w < (Chunk_T)g_heap_end;
         w = chunk_get_next_adjacent(w, g_heap_start, g_heap_end)) {

        if (!chunk_is_valid(w, g_heap_start, g_heap_end))
            return FALSE;
    }
    if (w) {
        fprintf(stderr, "Invalid chunk pointer at %p\n", (void *)w);
        return FALSE;
    }

    for (int i = 0; i < NUM_BINS; i++) {
        for (w = g_free_bins[i]; w; w = chunk_get_next_free_chunk(w)) {

            if (chunk_get_status(w) != CHUNK_FREE) {
                fprintf(stderr, "Non-free chunk in bin %d\n", i);
                return FALSE;
            }
            
            size_t units = chunk_get_units(w);
            int idx = get_bin_index(units);
            if (idx != i) {
                fprintf(stderr, "Invalid bin index: %d\n", idx);
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

            Chunk_T n = chunk_get_next_adjacent(w, g_heap_start, g_heap_end);
            if (n && n == chunk_get_next_free_chunk(w)) {
                fprintf(stderr, "Uncoalesced chunks in bin %d\n", i);
                return FALSE;
            }
        }
    }
    return TRUE;
}
#endif

/*--------------------------------------------------------------*/
/* size_to_units:                                               */
/* Returns capable number of units for 'size' bytes.            */
/*--------------------------------------------------------------*/
static size_t 
size_to_units(size_t size)
{
    return (size + (CHUNK_UNIT - 1)) / CHUNK_UNIT;
}

/*--------------------------------------------------------------*/
/* get_chunk_from_data_ptr:                                     */
/* Returns the header pointer that contains data 'm'.           */
/*--------------------------------------------------------------*/
static Chunk_T 
get_chunk_from_data_ptr(void *m)
{
    return (Chunk_T)((char *)m - CHUNK_UNIT);
}

/*--------------------------------------------------------------*/
/* get_bin_index:                                               */
/* Compute the bin index of given free block                    */
/* ceil(log(units))                                             */
/*--------------------------------------------------------------*/
static int 
get_bin_index(size_t units)
{  
    assert(units > 0);
    
    int idx = 0;
    units = units - 1;

    while (units > 0) {
        units >>= 1;
        idx++;
    }

    if (idx > (NUM_BINS - 1))
        idx = NUM_BINS - 1;

    return idx;
}

/*--------------------------------------------------------------------*/
/* init_my_heap:                                                      */
/* Initialize data structures and global variables for                */
/* chunk management.                                                  */
/*--------------------------------------------------------------------*/
static void 
init_my_heap(void)
{
    /* Initialize g_heap_start and g_heap_end */
    g_heap_start = g_heap_end = sbrk(0);
    if (g_heap_start == (void *)-1) {
        fprintf(stderr, "sbrk(0) failed\n");
        exit(-1);
    }
}

/*--------------------------------------------------------------*/
/* bin_insert_chunk:                                            */
/*   Insert a free chunk 'c' into the appropriate bin list.     */
/*   The chunk is inserted at the head of the list (LIFO).      */
/*--------------------------------------------------------------*/
static void 
bin_insert_chunk(Chunk_T c)
{
    assert(c >= (Chunk_T)g_heap_start && c < (Chunk_T)g_heap_end);
    assert(chunk_get_status(c) == CHUNK_FREE);
    assert(chunk_get_units(c) > 0);

    int idx = get_bin_index(chunk_get_units(c));
    Chunk_T g_head = g_free_bins[idx];

    chunk_set_next_free_chunk(c, g_head);
    chunk_set_prev_free_chunk(c, NULL);

    if (g_head)
        chunk_set_prev_free_chunk(g_head, c);
    
    g_free_bins[idx] = c;
}

/*--------------------------------------------------------------*/
/* bin_remove_chunk:                                            */
/*   Remove a free chunk 'c' from its current bin list.         */
/*   Updates the bin list pointers accordingly.                 */
/*--------------------------------------------------------------*/
static void
bin_remove_chunk(Chunk_T c)
{
    assert(c >= (Chunk_T)g_heap_start && c < (Chunk_T)g_heap_end);
    assert(chunk_get_status(c) == CHUNK_FREE);
    assert(chunk_get_units(c) > 0);

    int idx = get_bin_index(chunk_get_units(c));

    Chunk_T prev = chunk_get_prev_free_chunk(c);
    Chunk_T next = chunk_get_next_free_chunk(c);
    
    if (prev)
        chunk_set_next_free_chunk(prev, next);
    else
        g_free_bins[idx] = next;
    
    if (next)
        chunk_set_prev_free_chunk(next, prev);
    
    chunk_set_next_free_chunk(c, NULL);
    chunk_set_prev_free_chunk(c, NULL);
}

/*--------------------------------------------------------------*/
/* merge_chunk:                                                 */
/*   Merge two adjacent free chunks c1 and c2; return merged.   */
/*--------------------------------------------------------------*/
static Chunk_T merge_chunk(Chunk_T c1, Chunk_T c2)
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
static Chunk_T merge_chunks(Chunk_T c1, Chunk_T c2, Chunk_T c3)
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
static Chunk_T split_chunk(Chunk_T c, size_t units)
{
    size_t all_units = chunk_get_units(c);
    
    assert(c >= (Chunk_T)g_heap_start && c < (Chunk_T)g_heap_end);
    assert(all_units >= units + 3);
    assert(chunk_get_status(c) == CHUNK_FREE);

    chunk_set_units(c, all_units - units - 2);
    chunk_set_status(c, CHUNK_FREE);
    bin_insert_chunk(c);

    Chunk_T alloc = chunk_get_next_adjacent(c, g_heap_start, g_heap_end);
    chunk_set_units(alloc, units);
    chunk_set_status(alloc, CHUNK_IN_USE);

    return alloc;
}

/*--------------------------------------------------------------*/
/* allocate_more_memory:                                        */
/*   Expand heap by required units or 1024 units                */
/*   merge with previous chunk (if free)                        */
/*   return new allocated free chunk. (not inserted in list)    */
/*--------------------------------------------------------------*/
static Chunk_T allocate_more_memory(size_t units)
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
        bin_remove_chunk(prev_chunk);
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
    int idx;
    Chunk_T c = NULL;
    
    if (size <= 0) 
        return NULL;
    
    if (is_init == FALSE) { 
        init_my_heap();
        is_init = TRUE; 
    }

    assert(check_heap_validity());

    units = size_to_units(size);
    idx = get_bin_index(units);
    
    /* find suitable chunk */
    for (int i = idx; i < NUM_BINS; i++) {
        if (!g_free_bins[i])            
            continue;
        
        for (c = g_free_bins[i]; 
             c; 
             c = chunk_get_next_free_chunk(c)) {
        
            if (chunk_get_units(c) >= units) {
                bin_remove_chunk(c);
                
                if (chunk_get_units(c) >= units + 3)
                    c = split_chunk(c, units);
                
                chunk_set_status(c, CHUNK_IN_USE);
                assert(check_heap_validity());
                return (void *)((char *)c + CHUNK_UNIT);
            }
        }
    }

    // no suitable chunk found, allocate more memory
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
    return (void *)((char *)c + CHUNK_UNIT);
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

    Chunk_T next_chunk = chunk_get_next_adjacent(c, g_heap_start, g_heap_end);
    Chunk_T prev_chunk = chunk_get_prev_adjacent(c, g_heap_start, g_heap_end);

    unsigned int flag = 0;
    flag |= (next_chunk && chunk_get_status(next_chunk) == CHUNK_FREE);  /* bit0 */
    flag |= (prev_chunk && chunk_get_status(prev_chunk) == CHUNK_FREE) << 1;  /* bit1 */
    
    switch (flag) {
        case COALESCE_BOTH:
            bin_remove_chunk(prev_chunk);
            bin_remove_chunk(next_chunk);
            c = merge_chunks(prev_chunk, c, next_chunk);
            break;
        case COALESCE_LEFT:
            bin_remove_chunk(prev_chunk);
            c = merge_chunk(prev_chunk, c);
            break;
        case COALESCE_RIGHT:
            bin_remove_chunk(next_chunk);
            c = merge_chunk(c, next_chunk);
            break;
        case NO_COALESCE:
            break;
        default:
            fprintf(stderr, "Invalid coalescing flag: %u\n", flag);
            assert(0);
    }
    bin_insert_chunk(c);
    assert(check_heap_validity());
}
