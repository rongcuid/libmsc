#ifndef MSC_ALLOC_H_
#define MSC_ALLOC_H_

#include <stddef.h>

/**
 * @brief Allocator interface.
 */
struct msc_allocator {
  void *(*alloc)(struct msc_allocator *self, ptrdiff_t size, ptrdiff_t align,
                 ptrdiff_t count);
  void (*free)(struct msc_allocator *self, void *ptr, ptrdiff_t size,
               ptrdiff_t align);
  void *(*realloc)(struct msc_allocator *self, void *ptr, ptrdiff_t old_size,
                   ptrdiff_t new_size, ptrdiff_t align);
  void *ctx;
};

extern const struct msc_allocator msc_c_allocator;

#endif  // MSC_ALLOC_H_