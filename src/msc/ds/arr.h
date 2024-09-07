#ifndef MSC_DS_ARR_H_
#define MSC_DS_ARR_H_

#include <stddef.h>
#include <stdint.h>

#include "alloc/alloc.h"

enum msc_arr_e {
  MSC_ARR_OK = 0,
  MSC_ARR_ERR,
};

struct msc_arr {
  void *items;
  struct msc_allocator *alloc;
  ptrdiff_t len;
  ptrdiff_t cap;
  ptrdiff_t elem_size;
  ptrdiff_t align;
};

void msc_arr_init(struct msc_arr *self, ptrdiff_t elem_size,
                  ptrdiff_t elem_align, struct msc_allocator *alloc);

enum msc_arr_e msc_arr_init_with_cap(struct msc_arr *self, ptrdiff_t elem_size,
                                     ptrdiff_t elem_align, ptrdiff_t capacity,
                                     struct msc_allocator *alloc);
void msc_arr_deinit(struct msc_arr *self);

enum msc_arr_e msc_arr_push(struct msc_arr *self, void *item);
enum msc_arr_e msc_arr_pop(struct msc_arr *self, void *item);

#endif  // MSC_DS_ARR_H_