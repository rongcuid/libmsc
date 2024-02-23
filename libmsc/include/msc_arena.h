#ifndef MSC_ALLOC_H_
#define MSC_ALLOC_H_

#include <stddef.h>

struct msc_arena {
  void *begin;
  void *end;
};
typedef struct msc_arena_checkpoint {
  void *begin;
} msc_arena_checkpoint_t;

void msc_arena_init(struct msc_arena *arena, void *memory, size_t capacity);
void *msc_arena_alloc(struct msc_arena *arena, size_t align, size_t len,
                      size_t size);
msc_arena_checkpoint_t msc_arena_checkpoint(const struct msc_arena *arena);
void msc_arena_rewind(struct msc_arena *arena,
                      msc_arena_checkpoint_t checkpoint);

#endif  // MSC_ALLOC_H_