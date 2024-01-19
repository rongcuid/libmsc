#include <stdio.h>
#include <stdlib.h>
#include <unity/unity.h>

#include "msc_rng.h"

void setUp() {}
void tearDown() {}

void test_xors32_create() {
  mscrng_xors32_t rng;
  mscrng_xors32_init(&rng, 1);
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  return UNITY_END();
}
