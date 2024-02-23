#include "msc_arena.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

void msc_arena_init(struct msc_arena *arena, void *memory, size_t capacity) {
  arena->begin = memory;
  arena->end = memory + capacity;
}

void *msc_arena_alloc(struct msc_arena *arena, size_t align, size_t len,
                      size_t size) {
  void *ptr = NULL;
  if (align == 0) goto exit_param;
  // If align is not power of two, error
  if (align & (align - 1)) goto exit_param;
  uintptr_t begin_addr = (uintptr_t)arena->begin;
  uintptr_t align_mask = align - 1;
  ptrdiff_t padding = -begin_addr & align_mask;
  ptrdiff_t available = arena->end - arena->begin - padding;
  if (available < 0 || len > available / size) {
    goto exit_available;
  }
  ptr = arena->begin + padding;
  arena->begin += padding + len * size;
  memset(ptr, 0, len * size);
exit_available:;
exit_param:;
finally:
  return ptr;
}