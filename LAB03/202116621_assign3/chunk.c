/*--------------------------------------------------------------------*/
/* chunk.c                                                            */
/* Author: Donghwi Kim, KyoungSoo Park                                */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "chunk.h"

#define CHUNK_FOOTER(c) ((Chunk_T)((c) + 1 + (c)->units))

struct Chunk {
   Chunk_T next;       /* Pointer to the next chunk in the free chunk list */
   int units;          /* Capacity of a chunk (chunk units) */
   int status;         /* CHUNK_FREE or CHUNK_IN_USE */
};

/*--------------------------------------------------------------------*/
int
chunk_get_status(Chunk_T c)
{
   return c->status;
}
/*--------------------------------------------------------------------*/
void
chunk_set_status(Chunk_T c, int status)
{
   c->status = status;
}
/*--------------------------------------------------------------------*/
int
chunk_get_units(Chunk_T c)
{
   return c->units;
}
/*--------------------------------------------------------------------*/
void
chunk_set_units(Chunk_T c, int units)
{  
   c->units = units;
   CHUNK_FOOTER(c)->units = units;
}
/*--------------------------------------------------------------------*/
Chunk_T
chunk_get_next_free_chunk(Chunk_T c)
{
  return c->next;
}
/*--------------------------------------------------------------------*/
Chunk_T
chunk_get_prev_free_chunk(Chunk_T c)
{
  return CHUNK_FOOTER(c)->next;
}
/*--------------------------------------------------------------------*/
void
chunk_set_next_free_chunk(Chunk_T c, Chunk_T next)
{
   c->next = next;
}
/*--------------------------------------------------------------------*/
void
chunk_set_prev_free_chunk(Chunk_T c, Chunk_T prev)
{
   CHUNK_FOOTER(c)->next = prev;
}
/*--------------------------------------------------------------------*/
Chunk_T
chunk_get_next_adjacent(Chunk_T c, void* start, void* end)
{
   Chunk_T n;

   assert((void *)c >= start);

   n = c + c->units + 2;

   if ((void *)n >= end)
      return NULL;
   
   return n;
}
/*--------------------------------------------------------------------*/
Chunk_T
chunk_get_prev_adjacent(Chunk_T c, void* start, void* end)
{
   assert((void *)c >= start);
   
   Chunk_T n = c - ((c-1)->units) - 2;

   if ((void*)n < start)
      return NULL;

   return n;
}

#ifndef NDEBUG
/*--------------------------------------------------------------------*/
int 
chunk_is_valid(Chunk_T c, void *start, void *end)
/* Return 1 (TRUE) iff c is valid */
{
   assert(c != NULL);
   assert(start != NULL);
   assert(end != NULL);

   if (c < (Chunk_T)start)
      {fprintf(stderr, "Bad heap start\n"); return 0; }
   if (c >= (Chunk_T)end)
      {fprintf(stderr, "Bad heap end\n"); return 0; }
   if (c->units == 0)
      {fprintf(stderr, "Zero units\n"); return 0; }
   Chunk_T f = CHUNK_FOOTER(c);
   if (c->units != f->units)
      {fprintf(stderr, "Unit Mismatch\n"); return 0; }
   
   return 1;
}
#endif