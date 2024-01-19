#ifndef MSC_RNG_H_
#define MSC_RNG_H_

#include <stdint.h>

/**
 * @brief Xorshift from p. 4 of Marsaglia, "Xorshift RNGs"
 *
 * @param rng
 * @return uint32_t
 */
typedef struct mscrng_xors32 {
  uint32_t state;
} mscrng_xors32_t;
void mscrng_xors32_init(mscrng_xors32_t *rng, uint32_t seed);
uint32_t mscrng_xors32_rand(mscrng_xors32_t *rng);

typedef struct mscrng_xors64 {
  uint64_t state;
} mscrng_xors64_t;
void mscrng_xors64_init(mscrng_xors64_t *rng, uint64_t seed);
uint64_t mscrng_xors64_rand(mscrng_xors64_t *rng);

#endif // MSC_RNG_H_

#ifdef MSC_RNG_IMPLEMENTATION

void mscrng_xors32_init(mscrng_xors32_t *rng, uint32_t seed) {
  if (seed == 0)
    seed = -1;
  rng->state = seed;
}

uint32_t mscrng_xors32_rand(mscrng_xors32_t *rng) {
  uint32_t x = rng->state;
  x ^= x << 13;
  x ^= x << 17;
  x ^= x << 5;
  return rng->state = x;
}

#endif