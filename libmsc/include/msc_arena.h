#ifndef MSC_ALLOC_H_
#define MSC_ALLOC_H_

#include <stddef.h>

struct msca {
  void *begin;
  void *end;
};
typedef struct msca_cp {
  void *begin;
} msca_cp_t;

/**
 * @brief Initializes arena from memory region
 *
 * @param arena Arena to be initialized
 * @param memory Backing memory
 * @param capacity Backing memory capacity
 */
void msca_init(struct msca *arena, void *memory, size_t capacity);
/**
 * @brief Allocate memory from arena
 *
 * @param arena
 * @param align Must be power of two
 * @param len Number of elements
 * @param size Size of element
 * @return void* Pointer if success, NULL otherwise
 */
void *msca_try_alloc(struct msca *arena, size_t align, size_t len, size_t size);
/**
 * @brief Create a checkpoint
 *
 * @param arena
 * @return msca_cp_t
 */
msca_cp_t msca_checkpoint(const struct msca *arena);
/**
 * @brief Rewind to checkpoint, invalidating all allocations after checkpoint
 *
 * @param arena
 * @param checkpoint
 */
void msca_rewind(struct msca *arena, msca_cp_t checkpoint);
/**
 * @brief Splits arena into two, each half its original size
 *
 * @param arena Arena to be halved
 * @param other Other half of arena
 */
void msca_half(struct msca *arena, struct msca *other);

#endif  // MSC_ALLOC_H_