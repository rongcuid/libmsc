#ifndef MSC_ARENA_H_
#define MSC_ARENA_H_

#include <stddef.h>

/**
 * @brief A single-threaded, basic arena allocator that allocates from
 * bottom-up. The most basic allocator often used for scratchpads.
 */
typedef struct {
  char* begin;
  char* end;
} msc_arena_t;

/**
 * @brief Create an arena allocator from raw buffer.
 *
 * @param buf Pointer to buffer.
 * @param capacity Capacity of buffer.
 * @return msc_arena_t
 */
msc_arena_t msc_arena_from_buf(void* buf, ptrdiff_t capacity);

/**
 * @brief Allocate a smaller arena from the current arena.
 *
 * @param a Arena allocator.
 * @param capacity Capacity of the new allocator. Capacity is unchecked!
 * @return msc_arena_t
 */
msc_arena_t msc_arena_suballoc_unchecked(msc_arena_t* a, ptrdiff_t capacity);

void* msc_arena_malloc(msc_arena_t* a, ptrdiff_t size, ptrdiff_t align);
void msc_arena_free(msc_arena_t* a, void* ptr, ptrdiff_t size, ptrdiff_t align);
void* msc_arena_realloc(msc_arena_t* a, void* ptr, ptrdiff_t old_size,
                        ptrdiff_t old_align, ptrdiff_t new_size,
                        ptrdiff_t new_align);

#endif