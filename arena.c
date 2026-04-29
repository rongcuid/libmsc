#include "arena.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

mscc_arena_t mscc_arena_from_buf(void *buf, ptrdiff_t capacity) {
  mscc_arena_t a = {0};
  a.begin = buf;
  a.end = a.begin + capacity;
  return a;
}

mscc_arena_t mscc_arena_suballoc_unchecked(mscc_arena_t *a,
                                           ptrdiff_t capacity) {
  return mscc_arena_from_buf(mscc_arena_malloc(a, capacity, 1), capacity);
}

/**
    Credits: Chris Wellons:
    * https://nullprogram.com/blog/2023/09/27/
    * https://nullprogram.com/blog/2023/12/17/
    Modified.
 */
void *mscc_arena_malloc(mscc_arena_t *a, ptrdiff_t size, ptrdiff_t align) {
  // This arena allocator allocates from the end.
  ptrdiff_t available = a->end - a->begin;
  ptrdiff_t padding = -size & (align - 1);
  if (size > available - padding) {
    return NULL;
  }
  return a->end -= size + padding;
}

/**
    Credits: Chris Wellons:
    * https://nullprogram.com/blog/2023/09/27/
    * https://nullprogram.com/blog/2023/12/17/
    Modified.
 */
void mscc_arena_free(mscc_arena_t *a, void *ptr, ptrdiff_t size,
                     ptrdiff_t align) {
  // Only the last object reclaims memory when freed.
  if (ptr == a->end) {
    ptrdiff_t padding = -size & (align - 1);
    a->end += size + padding;
  }
}

void *mscc_arena_realloc(mscc_arena_t *a, void *ptr, ptrdiff_t old_size,
                         ptrdiff_t old_align, ptrdiff_t new_size,
                         ptrdiff_t new_align) {
  if (new_size < old_size) {
    return NULL;
  }
  // If the object is the most recently allocated object (i.e. pointer equal to
  // the end pointer), reclaim space.
  if (ptr == a->end) {
    ptrdiff_t old_padding = -old_size & (old_align - 1);
    char *old_end = a->end;

    a->end += old_size + old_padding;

    void *r = mscc_arena_malloc(a, new_size, new_align);
    if (r == NULL) {
      a->end = old_end;
      return NULL;
    }
    return memmove(r, ptr, old_size);
  }

  void *r = mscc_arena_malloc(a, new_size, new_align);
  if (r == NULL) {
    return NULL;
  }
  return memcpy(r, ptr, old_size);
}
