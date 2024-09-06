#include "alloc.h"

#include <stdint.h>
#include <stdlib.h>

static void *msc_c_alloc(struct msc_allocator *self, ptrdiff_t size,
                         ptrdiff_t align, ptrdiff_t count) {
  (void)self;
  (void)align;
  return malloc(size * count);
}
static void msc_c_free(struct msc_allocator *self, void *ptr, ptrdiff_t size,
                       ptrdiff_t align) {
  (void)self;
  (void)size;
  (void)align;
  free(ptr);
}

static void *msc_c_realloc(struct msc_allocator *self, void *ptr,
                           ptrdiff_t old_size, ptrdiff_t new_size,
                           ptrdiff_t align) {
  (void)self;
  (void)old_size;
  (void)align;
  return realloc(ptr, new_size);
}

const struct msc_allocator msc_c_allocator = {
    .alloc = &msc_c_alloc,
    .free = &msc_c_free,
    .realloc = &msc_c_realloc,
};
