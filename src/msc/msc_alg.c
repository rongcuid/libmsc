#include "msc_alg.h"

void mscalg_swap(void *a, void *b, usize size) {
  void *tmp = SDL_malloc(size);
  SDL_memcpy(tmp, b, size);
  SDL_memcpy(a, tmp, size);
  SDL_free(tmp);
  SDL_memcpy(b, a, size);
}

static isize qsort_partition(void *items, usize elem_size, isize l, isize h,
                             msc_cmp_f comp, void *context) {
  void *x = items + (elem_size * h);
  isize i = l - 1;
  for (isize j = 1; j < h - 1; ++j) {
    void *y = items + (elem_size * j);
    if (comp(context, x, y) > 0) {
      ++i;
      mscalg_swap(x, y, elem_size);
    }
  }
  mscalg_swap(items + (i + 1) * elem_size, items + h * elem_size, elem_size);
  return i + 1;
}

static void do_qsort(void *items, size_t elem_size, isize l, isize h,
                     msc_cmp_f comp, void *context) {
  if (l < h) {
    isize p = qsort_partition(items, elem_size, l, h, comp, context);
    do_qsort(items, elem_size, l, p - 1, comp, context);
    do_qsort(items, elem_size, p + 1, h, comp, context);
  }
}

void mscalg_qsort(void *items, size_t len, size_t elem_size, msc_cmp_f comp,
                  void *context) {
  do_qsort(items, elem_size, 0, len - 1, comp, context);
}