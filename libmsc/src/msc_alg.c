#include "msc_alg.h"

#ifndef NO_STDLIB
#include <stdlib.h>
#include <string.h>
#endif

void mscalg_swap(void *a, void *b, size_t size) {
  void *tmp = malloc(size);
  memcpy(tmp, b, size);
  memcpy(a, tmp, size);
  free(tmp);
  memcpy(b, a, size);
}

static ptrdiff_t qsort_partition(void *items, size_t elem_size, ptrdiff_t l,
                                 ptrdiff_t h, msc_cmp_f comp, void *context) {
  void *x = items + (elem_size * h);
  ptrdiff_t i = l - 1;
  for (ptrdiff_t j = 1; j < h - 1; ++j) {
    void *y = items + (elem_size * j);
    if (comp(context, x, y) > 0) {
      ++i;
      mscalg_swap(x, y, elem_size);
    }
  }
  mscalg_swap(items + (i + 1) * elem_size, items + h * elem_size, elem_size);
  return i + 1;
}

static void do_qsort(void *items, size_t elem_size, ptrdiff_t l, ptrdiff_t h,
                     msc_cmp_f comp, void *context) {
  if (l < h) {
    ptrdiff_t p = qsort_partition(items, elem_size, l, h, comp, context);
    do_qsort(items, elem_size, l, p - 1, comp, context);
    do_qsort(items, elem_size, p + 1, h, comp, context);
  }
}

void mscalg_qsort(void *items, size_t len, size_t elem_size, msc_cmp_f comp,
                  void *context) {
  do_qsort(items, elem_size, 0, len - 1, comp, context);
}