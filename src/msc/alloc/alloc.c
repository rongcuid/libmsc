#include "alloc.h"

#include <stdint.h>
#include <stdlib.h>

static void *msc_c_alloc(struct msc_allocator *self, ptrdiff_t size,
                         ptrdiff_t align, ptrdiff_t count) {
  (void)self;
  (void)align;
  return malloc(size * count);
}
static void msc_c_free(struct msc_allocator *self, void *ptr, ptrdiff_t size) {
  (void)self;
  (void)size;
  free(ptr);
}

static void *msc_c_realloc(struct msc_allocator *self, void *ptr,
                           ptrdiff_t old_size, ptrdiff_t new_size) {
  (void)self;
  (void)old_size;
  return realloc(ptr, new_size);
}

const struct msc_allocator msc_c_allocator = {
    .alloc = &msc_c_alloc,
    .free = &msc_c_free,
    .realloc = &msc_c_realloc,
};

void msc_arena_init(struct msc_arena *self, char *buffer, ptrdiff_t capacity) {
  self->begin = buffer;
  self->end = self->begin ? self->begin + capacity : NULL;
}

void *msc_arena_alloc(struct msc_arena *self, ptrdiff_t size, ptrdiff_t align,
                      ptrdiff_t count) {
  void *result = NULL;
  ptrdiff_t padding = -(uintptr_t)self->begin & (align - 1);
  ptrdiff_t available = self->end - self->begin - padding;
  ptrdiff_t request_size = count * size;
  if (available < 0 || request_size > available) {
    goto end;
  }
  result = self->begin + padding;
  self->begin += padding + request_size;
end:
  return result;
}

void msc_arena_free(struct msc_arena *self, void *ptr, ptrdiff_t size) {}

void *msc_arena_realloc(struct msc_arena *self, void *ptr, ptrdiff_t old_size,
                        ptrdiff_t new_size) {
  return ptr;
}

static inline void *arena_alloc(struct msc_allocator *self, ptrdiff_t size,
                                ptrdiff_t align, ptrdiff_t count) {
  return msc_arena_alloc(self->ctx, size, align, count);
}

static inline void arena_free(struct msc_allocator *self, void *ptr,
                              ptrdiff_t size) {
  msc_arena_free(self->ctx, ptr, size);
}

static inline void *arena_realloc(struct msc_allocator *self, void *ptr,
                                  ptrdiff_t old_size, ptrdiff_t new_size) {
  return msc_arena_realloc(self->ctx, ptr, old_size, new_size);
}

void msc_arena_to_allocator(struct msc_arena *self,
                            struct msc_allocator *alloc) {
  *alloc = (struct msc_allocator){
      .alloc = &arena_alloc,
      .free = &arena_free,
      .realloc = &arena_realloc,
      .ctx = self,
  };
}