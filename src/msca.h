#ifndef MSCA_H__
#define MSCA_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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

/**
 * @brief Callback structure for releasing a buffer
 * @param context Arbitrary data for the release callback
 * @param data Data to be released
 */
typedef void (*msca_release_f)(void *context, void *data);
/**
 * @brief A releaser which uses releases data allocated by `malloc()`
 */
extern msca_release_f msca_malloc_release;

/**
 * @brief A buffer release callback which calls `free(buf)`
 *
 * @param context
 * @param buf
 */
void msca_buf_release_free(void *context, void *buf);

typedef enum msca_result {
  MSCA_OK = 0,
  MSCA_ERR,
  MSCA_NOMEM,
} msca_result_t;

/**
 * @brief Create a null array. No allocations. Error is impossible.
 *
 * @param length Logical length of array
 * @param array Output array
 */
void msca_mknull(size_t length, struct ArrowArray *array);

/**
 * @brief Create an untyped primitive array, taking ownership of buffers.
 *
 * @param length Logical length of array
 * @param validity Validity buffer (bitmap)
 * @param data Data buffer
 * @param validity_release Release callback, nullable.
 * @param validity_release_context Passed as first argument to
 * `release_validity`
 * @param data_release Releasese callback, nullable.
 * @param data_release_context Passed as first argument to `release_data`
 * @param array Output array
 * @return msca_result_t
 */
msca_result_t msca_mkprim(size_t length, uint8_t *validity, void *data,
                          msca_release_f validity_release,
                          void *validity_release_context,
                          msca_release_f data_release,
                          void *data_release_context, struct ArrowArray *array);

#endif // MSCA_H__

#ifdef MSCA_IMPLEMENTATION

static void msca_malloc_release_(void *context, void *data) { free(data); }

msca_release_f msca_malloc_release = &msca_malloc_release_;

static void msca_release_null(struct ArrowArray *arr) { arr->release = NULL; }

void msca_mknull(size_t length, struct ArrowArray *array) {
  *array = (struct ArrowArray){0};
  array->length = length;
  array->null_count = length;
  array->release = msca_release_null;
}

typedef struct msca_prim_priv {
  msca_release_f data_release;
  void *data_release_context;
  msca_release_f validity_release;
  void *validity_release_context;
  const void *buffers[2];
} msca_prim_priv_t;

static void msca_release_prim(struct ArrowArray *arr) {
  msca_prim_priv_t *priv = (msca_prim_priv_t *)arr->private_data;
  if (priv->validity_release) {
    priv->validity_release(priv->validity_release_context,
                           (void *)arr->buffers[0]);
  }
  if (priv->data_release) {
    priv->data_release(priv->data_release_context, (void *)arr->buffers[1]);
  }
  free(priv);
  arr->release = NULL;
}

msca_result_t msca_mkprim(size_t length, uint8_t *validity, void *data,
                          msca_release_f validity_release,
                          void *validity_release_context,
                          msca_release_f data_release,
                          void *data_release_context,
                          struct ArrowArray *array) {
  msca_result_t result = MSCA_ERR;
  // Private data
  msca_prim_priv_t *priv = (msca_prim_priv_t *)malloc(sizeof(*priv));
  if (!priv) {
    result = MSCA_NOMEM;
    goto finally;
  }
  *priv = (msca_prim_priv_t){0};
  priv->data_release = data_release;
  priv->data_release_context = data_release_context;
  priv->validity_release = validity_release;
  priv->validity_release_context = validity_release_context;
  priv->buffers[0] = validity;
  priv->buffers[1] = data;
  // Populate array
  *array = (struct ArrowArray){
      .length = length,
      .null_count = validity == NULL ? 0 : -1,
      .n_buffers = 2,
      .buffers = priv->buffers,
      .release = msca_release_prim,
      .private_data = priv,
  };
  goto ok;
  // Exit point
err_after_priv:
  free(priv);
  goto finally;
ok:
  result = MSCA_OK;
finally:
  return result;
}

#endif