#ifndef MSC_ARENA_H_
#define MSC_ARENA_H_

#include "alloc.h"

/**
 * @brief Arena allocator. Do not access fields directly.
 */
struct msc_arena {
  char *begin;
  char *end;
};

void msc_arena_init(struct msc_arena *self, char *buffer, ptrdiff_t capacity);
void *msc_arena_alloc(struct msc_arena *self, ptrdiff_t size, ptrdiff_t align,
                      ptrdiff_t count);
void msc_arena_free(struct msc_arena *self, void *ptr, ptrdiff_t size,
                    ptrdiff_t align);
void *msc_arena_realloc(struct msc_arena *self, void *ptr, ptrdiff_t old_size,
                        ptrdiff_t new_size, ptrdiff_t align);
void msc_arena_to_allocator(struct msc_arena *self,
                            struct msc_allocator *alloc);

#endif  // MSC_ARENA_H_