#include <stdio.h>
#include <unity/unity.h>

#include "msca.h"

void setUp(void) {}

void test_null_arr_create_release() {
  struct ArrowArray nullarr;
  msca_mknull(1000, &nullarr);
  TEST_ASSERT_NOT_NULL(nullarr.release);
  TEST_ASSERT_EQUAL(nullarr.null_count, 1000);
  TEST_ASSERT_EQUAL(nullarr.length, 1000);
  TEST_ASSERT_EQUAL(nullarr.n_buffers, 0);
  TEST_ASSERT_EQUAL(nullarr.n_children, 0);
  nullarr.release(&nullarr);
  TEST_ASSERT_NULL(nullarr.release);
}

void test_int_arr_create_release() {
  int *ints = calloc(1000, sizeof(*ints));
  TEST_ASSERT_EACH_EQUAL_INT(0, ints, 1000);
  struct ArrowArray intarr;
  msca_mkprim(1000, NULL, ints, NULL, &msca_malloc_releaser, &intarr);
  TEST_ASSERT_NOT_NULL(intarr.release);
  TEST_ASSERT_EQUAL(intarr.length, 1000);
  TEST_ASSERT_EQUAL(intarr.n_buffers, 2);
  TEST_ASSERT_EQUAL(intarr.n_children, 0);
  TEST_ASSERT_EACH_EQUAL_INT(0, intarr.buffers[1], 1000);
  intarr.release(&intarr);
  TEST_ASSERT_NULL(intarr.release);
}

void tearDown(void) {}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_null_arr_create_release);
  RUN_TEST(test_int_arr_create_release);
  return UNITY_END();
}
