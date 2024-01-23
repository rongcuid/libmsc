#ifndef MSC_ALG_H_
#define MSC_ALG_H_

#include <stddef.h>
#include <stdint.h>

typedef ptrdiff_t (*msc_cmp_f)(const void *context, const void *a,
                               const void *b);

void mscalg_swap(void *a, void *b, size_t size);
void mscalg_qsort(void *items, size_t len, size_t elem_size, msc_cmp_f comp,
                  void *context);

#endif  // MSC_ALG_H_