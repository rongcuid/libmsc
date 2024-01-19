#ifndef MSC_ALG_H_
#define MSC_ALG_H_

typedef isize (*msc_cmp_f)(const void *context, const void *a, const void *b);

void mscalg_swap(void *a, void *b, usize size);
void mscalg_qsort(void *items, usize len, usize elem_size, msc_cmp_f comp,
                  void *context);

#endif // MSC_ALG_H_