#ifndef MSC_ALG_H_
#define MSC_ALG_H_

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Swaps two segments of memory. Allocates `size` amount of temporary
 * buffer.
 */
void mscalg_swap(void *a, void *b, size_t size);

/**
 * @brief Comparison function for sorts.
 *
 * @param context Arbitrary user pointer
 * @return If a < b, less than 0; if equal, return 0; otherwise, greater than 0
 */
typedef ptrdiff_t (*msc_cmp_f)(const void *context, const void *a,
                               const void *b);

/**
 * @brief Use quick sort to sort `items`
 *
 * @param items Pointer to array
 * @param len Length of array
 * @param elem_size Size of each element
 * @param comp Comparison function
 * @param context Passed as first argument to comparison function
 */
void mscalg_qsort(void *items, size_t len, size_t elem_size, msc_cmp_f comp,
                  void *context);

#endif  // MSC_ALG_H_