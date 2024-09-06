#ifndef MSC_ALLOC_H_
#define MSC_ALLOC_H_

#include <stddef.h>

struct msc_alloc {
  void *(*alloc)(ptrdiff_t size, void *ctx);
  void (*free)(void *ptr, ptrdiff_t size, void *ctx);
  void *(*realloc)(void *ptr, ptrdiff_t old_size, ptrdiff_t new_size,
                   void *ctx);
  void *ctx;
};

#endif // MSC_ALLOC_H_