#include "arena.h"

#include <stdint.h>
#include <string.h>

void msc_arena_init(struct msc_arena *self, char *buffer, ptrdiff_t capacity) {
  self->begin = buffer;
  self->end = self->begin ? self->begin + capacity : NULL;
}

void *msc_arena_alloc(struct msc_arena *self, ptrdiff_t size, ptrdiff_t align,
                      ptrdiff_t count) {
  void *result = NULL;
  ptrdiff_t padding = -size & (align - 1);
  ptrdiff_t available = self->end - self->begin - padding;
  ptrdiff_t req_size = count * size;
  if (available < 0 || req_size > available) {
    goto end;
  }
  // Allocate in reverse so self->end always point to last allocation
  self->end -= req_size + padding;
  result = self->end;
end:
  return result;
}

void msc_arena_free(struct msc_arena *self, void *ptr, ptrdiff_t size,
                    ptrdiff_t align) {
  // If pointer is the last allocation
  if (ptr == self->end) {
    ptrdiff_t padding = -size & (align - 1);
    self->end += size + padding;
  }
}

void *msc_arena_realloc(struct msc_arena *self, void *ptr, ptrdiff_t old_size,
                        ptrdiff_t new_size, ptrdiff_t align) {
  void *result = NULL;
  if (ptr == self->end) {
    ptrdiff_t new_padding = -new_size & (align - 1);
    ptrdiff_t available = self->end - self->begin;
    // If pointer is last allocation, extend the allocation
    ptrdiff_t delta_size = new_size + new_padding - old_size;
    if (available < 0 || delta_size > available) {
      goto end;
    }
    self->end -= delta_size;
    result = self->end;
  } else {
    // Otherwise, try to make a new allocation
    result = msc_arena_alloc(self, new_size, align, 1);
  }
  if (result) {
    memmove(result, ptr, old_size);
  }
end:
  return result;
}

static inline void *arena_alloc(struct msc_allocator *self, ptrdiff_t size,
                                ptrdiff_t align, ptrdiff_t count) {
  return msc_arena_alloc(self->ctx, size, align, count);
}

static inline void arena_free(struct msc_allocator *self, void *ptr,
                              ptrdiff_t size, ptrdiff_t align) {
  msc_arena_free(self->ctx, ptr, size, align);
}

static inline void *arena_realloc(struct msc_allocator *self, void *ptr,
                                  ptrdiff_t old_size, ptrdiff_t new_size,
                                  ptrdiff_t align) {
  return msc_arena_realloc(self->ctx, ptr, old_size, new_size, align);
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