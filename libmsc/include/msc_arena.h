#ifndef MSC_ALLOC_H_
#define MSC_ALLOC_H_

#include <stddef.h>

struct msca {
  void *begin;
  void *end;
};
typedef struct msca_cp {
  void *begin;
} msca_cp_t;

void msca_init(struct msca *arena, void *memory, size_t capacity);
void *msca_alloc(struct msca *arena, size_t align, size_t len, size_t size);
msca_cp_t msca_checkpoint(const struct msca *arena);
void msca_rewind(struct msca *arena, msca_cp_t checkpoint);

#endif  // MSC_ALLOC_H_