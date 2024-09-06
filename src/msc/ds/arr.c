#include "arr.h"

void msc_arr_init(struct msc_arr *self, ptrdiff_t elem_size,
                  ptrdiff_t elem_align, struct msc_allocator *alloc) {
  struct msc_arr a = {0};
  a.alloc = alloc;
  a.elem_size = elem_size;
  a.align = elem_align;
  *self = a;
}

enum msc_arr_e msc_arr_init_with_cap(struct msc_arr *self, ptrdiff_t elem_size,
                                     ptrdiff_t elem_align, ptrdiff_t capacity,
                                     struct msc_allocator *alloc) {
  enum msc_arr_e err = MSC_ARR_ERR;
  struct msc_arr a = {0};
  msc_arr_init(&a, elem_size, elem_align, alloc);
  a.cap = capacity;
  a.items = alloc->alloc(alloc, elem_size, elem_align, capacity);
  if (!a.items) goto end;
  goto ok;
ok:
  *self = a;
  err = MSC_ARR_OK;
end:
  return err;
}

void msc_arr_deinit(struct msc_arr *self) {
  self->alloc->free(self->alloc, self->items, self->elem_size * self->cap,
                    self->align);
  *self = (struct msc_arr){0};
}