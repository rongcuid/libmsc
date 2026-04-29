#include "alloc.h"

#include <stdlib.h>

#define CHECK(pred)                                                            \
  if (!(pred)) {                                                               \
    return nullptr;                                                            \
  }

void *mscc_malloc(const mscc_allocator_t *alloc, ptrdiff_t size,
                  ptrdiff_t align) {
  CHECK(alloc != nullptr);
  CHECK(alloc->malloc != nullptr);
  CHECK(size >= 0);
  CHECK(align >= 0);
  return alloc->malloc(alloc->context, size, align);
}

void *mscc_realloc(const mscc_allocator_t *alloc, void *ptr, ptrdiff_t old_size,
                   ptrdiff_t old_align, ptrdiff_t new_size,
                   ptrdiff_t new_align) {
  CHECK(alloc != nullptr);
  CHECK(alloc->realloc != nullptr);
  CHECK(old_size >= 0);
  CHECK(old_align >= 0);
  CHECK(new_size >= old_size);
  CHECK(new_align >= 0);
  return alloc->realloc(alloc->context, ptr, old_size, old_align, new_size,
                        new_align);
}

void mscc_free(const mscc_allocator_t *alloc, void *ptr, ptrdiff_t size,
               ptrdiff_t align) {
  if (alloc && alloc->free) {
    alloc->free(alloc->context, ptr, size, align);
  }
}

#undef CHECK

static void *c_malloc(void *context, ptrdiff_t size, ptrdiff_t align) {
  (void)context;
  (void)align;
  return malloc(size);
}

static void *c_realloc(void *context, void *ptr, ptrdiff_t old_size,
                       ptrdiff_t old_align, ptrdiff_t new_size,
                       ptrdiff_t new_align) {
  (void)context;
  (void)old_size;
  (void)old_align;
  (void)new_align;
  return realloc(ptr, new_size);
}

static void c_free(void *context, void *ptr, ptrdiff_t size, ptrdiff_t align) {
  (void)context;
  (void)size;
  (void)align;
  free(ptr);
}

const mscc_allocator_t mscc_c_allocator = {
    .malloc = &c_malloc,
    .realloc = &c_realloc,
    .free = &c_free,
};