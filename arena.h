#ifndef MSCC_ARENA_H_
#define MSCC_ARENA_H_

#include <stddef.h>

/**
 * @brief A single-threaded, basic arena allocator that allocates from
 * bottom-up. The most basic allocator often used for scratchpads.
 */
typedef struct {
  char *begin;
  char *end;
} mscc_arena_t;

/**
 * @brief Create an arena allocator from raw buffer.
 *
 * @param buf Pointer to buffer.
 * @param capacity Capacity of buffer.
 * @return mscc_arena_t
 */
mscc_arena_t mscc_arena_from_buf(void *buf, ptrdiff_t capacity);

/**
 * @brief Allocate a smaller arena from the current arena.
 *
 * @param a Arena allocator.
 * @param capacity Capacity of the new allocator. Capacity is unchecked!
 * @return mscc_arena_t
 */
mscc_arena_t mscc_arena_suballoc_unchecked(mscc_arena_t *a, ptrdiff_t capacity);

void *mscc_arena_malloc(mscc_arena_t *a, ptrdiff_t size, ptrdiff_t align);
void mscc_arena_free(mscc_arena_t *a, void *ptr, ptrdiff_t size,
                     ptrdiff_t align);
void *mscc_arena_realloc(mscc_arena_t *a, void *ptr, ptrdiff_t old_size,
                         ptrdiff_t old_align, ptrdiff_t new_size,
                         ptrdiff_t new_align);

#endif