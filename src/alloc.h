#ifndef MSCC_ALLOC_H_
#define MSCC_ALLOC_H_

#include <stddef.h>

/**
 * @brief A function pointer to `malloc` operator for an allocator.
 * Sizes are specified in `ptrdiff_t` due to being signed and expresses the
 * difference between base pointer and end of object.
 *
 * @param context Context pointer as specified in the `context` field of
 * `mscc_allocator_t`.
 * @param size Size of object.
 * @param align Alignment requirement of object.
 * @return Pointer to reallocated object, or nullptr if failed.
 */
typedef void *(*mscc_malloc_pfn)(void *context, ptrdiff_t size,
                                 ptrdiff_t align);
/**
 * @brief A function pointer to `realloc` operator for an allocator.
 * Sizes are specified in `ptrdiff_t` due to being signed and expresses the
 * difference between base pointer and end of object.
 *
 * @param context Context pointer as specified in the `context` field of
 * `mscc_allocator_t`.
 * @param ptr Original pointer.
 * @param old_size Size of original object.
 * @param old_align Alignment of original object.
 * @param new_size Size of new object.
 * @param new_align Alignment of new object.
 * @return Pointer to reallocated object, or nullptr if failed.
 */
typedef void *(*mscc_realloc_pfn)(void *context, void *ptr, ptrdiff_t old_size,
                                  ptrdiff_t old_align, ptrdiff_t new_size,
                                  ptrdiff_t new_align);

/**
 * @brief A function pointer to `free` operator for an allocator.
 * Sizes are specified in `ptrdiff_t` due to being signed and expresses the
 * difference between base pointer and end of object.
 *
 * @param context Context pointer as specified in the `context` field of
 * `mscc_allocator_t`.
 * @param ptr Object to free.
 * @param size Size of object.
 * @param align Alignment of object.
 */
typedef void (*mscc_free_pfn)(void *context, void *ptr, ptrdiff_t size,
                              ptrdiff_t align);

typedef struct {
  void *context;
  mscc_malloc_pfn malloc;
  mscc_realloc_pfn realloc;
  mscc_free_pfn free;
} mscc_allocator_t;

/**
 * @brief The default C stdlib allocator.
 */
extern const mscc_allocator_t mscc_c_allocator;

void *mscc_malloc(const mscc_allocator_t *alloc, ptrdiff_t size,
                  ptrdiff_t align);
void *mscc_realloc(const mscc_allocator_t *alloc, void *ptr, ptrdiff_t old_size,
                   ptrdiff_t old_align, ptrdiff_t new_size,
                   ptrdiff_t new_align);
void mscc_free(const mscc_allocator_t *alloc, void *ptr, ptrdiff_t size,
               ptrdiff_t align);

#endif