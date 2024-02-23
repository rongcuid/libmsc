#ifndef MSC_ALLOC_H_
#define MSC_ALLOC_H_

#include <stddef.h>

struct msc_arena {
  void *begin;
  void *end;
};

void msc_arena_init(struct msc_arena *arena, void *memory, size_t capacity);
void *msc_arena_alloc(struct msc_arena *arena, size_t align, size_t len,
                      size_t size);

#endif  // MSC_ALLOC_H_