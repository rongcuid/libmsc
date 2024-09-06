#ifndef MSC_ALLOC_H_
#define MSC_ALLOC_H_

#include <stddef.h>

/**
 * @brief Allocator interface.
 *
 */
struct msc_allocator {
  void *(*alloc)(struct msc_allocator *self, ptrdiff_t size, ptrdiff_t align,
                 ptrdiff_t count);
  void (*free)(struct msc_allocator *self, void *ptr, ptrdiff_t size);
  void *(*realloc)(struct msc_allocator *self, void *ptr, ptrdiff_t old_size,
                   ptrdiff_t new_size);
  void *ctx;
};

extern const struct msc_allocator msc_c_allocator;

/**
 * @brief Arena allocator. Do not access fields directly.
 *
 */
struct msc_arena {
  char *begin;
  char *end;
};

void msc_arena_init(struct msc_arena *self, char *buffer, ptrdiff_t capacity);
void *msc_arena_alloc(struct msc_arena *self, ptrdiff_t size, ptrdiff_t align,
                      ptrdiff_t count);
void msc_arena_free(struct msc_arena *self, void *ptr, ptrdiff_t size);
void *msc_arena_realloc(struct msc_arena *self, void *ptr, ptrdiff_t old_size,
                        ptrdiff_t new_size);
void msc_arena_to_allocator(struct msc_arena *self,
                            struct msc_allocator *alloc);

#endif // MSC_ALLOC_H_