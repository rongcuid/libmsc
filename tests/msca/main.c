#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unity/unity.h>

#include "msca.h"

// The following is the canonical definition of arrow C data interface
// structures
#ifndef ARROW_C_DATA_INTERFACE
#define ARROW_C_DATA_INTERFACE

#define ARROW_FLAG_DICTIONARY_ORDERED 1
#define ARROW_FLAG_NULLABLE 2
#define ARROW_FLAG_MAP_KEYS_SORTED 4

struct ArrowSchema {
  // Array type description
  const char *format;
  const char *name;
  const char *metadata;
  int64_t flags;
  int64_t n_children;
  struct ArrowSchema **children;
  struct ArrowSchema *dictionary;

  // Release callback
  void (*release)(struct ArrowSchema *);
  // Opaque producer-specific data
  void *private_data;
};

struct ArrowArray {
  // Array data description
  int64_t length;
  int64_t null_count;
  int64_t offset;
  int64_t n_buffers;
  int64_t n_children;
  const void **buffers;
  struct ArrowArray **children;
  struct ArrowArray *dictionary;

  // Release callback
  void (*release)(struct ArrowArray *);
  // Opaque producer-specific data
  void *private_data;
};

#endif // ARROW_C_DATA_INTERFACE

// The following is the canonical definition of Arrow C stream interface
// structures
#ifndef ARROW_C_STREAM_INTERFACE
#define ARROW_C_STREAM_INTERFACE

struct ArrowArrayStream {
  // Callbacks providing stream functionality
  int (*get_schema)(struct ArrowArrayStream *, struct ArrowSchema *out);
  int (*get_next)(struct ArrowArrayStream *, struct ArrowArray *out);
  const char *(*get_last_error)(struct ArrowArrayStream *);

  // Release callback
  void (*release)(struct ArrowArrayStream *);

  // Opaque producer-specific data
  void *private_data;
};

#endif // ARROW_C_STREAM_INTERFACE

void setUp(void) {}

void test_null_arr_create_release() {
  struct ArrowArray nullarr;
  msca_mknull(&nullarr, 1000);
  TEST_ASSERT_NOT_NULL(nullarr.release);
  TEST_ASSERT_EQUAL(nullarr.null_count, 1000);
  TEST_ASSERT_EQUAL(nullarr.length, 1000);
  TEST_ASSERT_EQUAL(nullarr.n_buffers, 0);
  TEST_ASSERT_EQUAL(nullarr.n_children, 0);
  nullarr.release(&nullarr);
  TEST_ASSERT_NULL(nullarr.release);
}

void test_int_arr_create_release() {
  msca_buf_t ints;
  TEST_ASSERT_EQUAL(msca_malloc(&ints, 1000 * sizeof(int32_t)), MSCA_OK);
  TEST_ASSERT_NOT_NULL(ints.release);
  TEST_ASSERT_NULL(ints.release_context);
  memset(ints.data, 0, 1000 * sizeof(int32_t));
  TEST_ASSERT_EACH_EQUAL_INT(0, ints.data, 1000);
  struct ArrowArray intarr;
  msca_mkprim(&intarr, 1000, NULL, &ints);
  TEST_ASSERT_NOT_NULL(intarr.release);
  TEST_ASSERT_EQUAL(intarr.length, 1000);
  TEST_ASSERT_EQUAL(intarr.n_buffers, 2);
  TEST_ASSERT_EQUAL(intarr.n_children, 0);
  TEST_ASSERT_EACH_EQUAL_INT(0, intarr.buffers[1], 1000);
  intarr.release(&intarr);
  TEST_ASSERT_NULL(intarr.release);
}

void test_int_arr_badargs() {
  struct ArrowArray arr = {0};
  TEST_ASSERT_EQUAL(msca_mkprim(&arr, 0, NULL, NULL), MSCA_BADARGS);
}

void tearDown(void) {}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_null_arr_create_release);
  RUN_TEST(test_int_arr_create_release);
  RUN_TEST(test_int_arr_badargs);
  return UNITY_END();
}
