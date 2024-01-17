#ifndef MSCA_H__
#define MSCA_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/_types/_null.h>

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
 */
typedef struct msca_releaser {
  void (*release)(struct msca_releaser *releaser, void *data);
  void *context;
} msca_releaser_t;

/**
 * @brief A releaser which uses releases data allocated by `malloc()`
 */
extern msca_releaser_t msca_malloc_releaser;

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
 * @brief Create an untyped primitive array.
 *
 * @param length Logical length of array
 * @param validity Validity buffer (bitmap)
 * @param data Data buffer
 * @param validity_releaser Release callback. If specified, takes ownership of
 * validity buffer
 * @param data_releaser Releasese callback. If specified, takes ownership of
 * data buffer
 * @param array Output array
 * @return msca_result_t
 */
msca_result_t msca_mkprim(size_t length, uint8_t *validity, void *data,
                          msca_releaser_t *validity_releaser,
                          msca_releaser_t *data_releaser,
                          struct ArrowArray *array);

#endif // MSCA_H__

#ifdef MSCA_IMPLEMENTATION

static void msca_malloc_releaser_release(msca_releaser_t *releaser,
                                         void *data) {
  free(data);
}

msca_releaser_t msca_malloc_releaser = {
    .release = msca_malloc_releaser_release,
};

static void msca_release_null(struct ArrowArray *arr) { arr->release = NULL; }

void msca_mknull(size_t length, struct ArrowArray *array) {
  *array = (struct ArrowArray){0};
  array->length = length;
  array->null_count = length;
  array->release = msca_release_null;
}

typedef struct msca_prim_priv {
  msca_releaser_t *data_releaser;
  msca_releaser_t *nulls_releaser;
  const void *buffers[2];
} msca_prim_priv_t;

static void msca_release_prim(struct ArrowArray *arr) {
  msca_prim_priv_t *priv = (msca_prim_priv_t *)arr->private_data;
  if (priv->nulls_releaser) {
    priv->nulls_releaser->release(priv->nulls_releaser,
                                  (void *)arr->buffers[0]);
  }
  if (priv->data_releaser) {
    priv->data_releaser->release(priv->data_releaser, (void *)arr->buffers[1]);
  }
  free(priv);
  arr->release = NULL;
}

msca_result_t msca_mkprim(size_t length, uint8_t *nulls, void *data,
                          msca_releaser_t *nulls_releaser,
                          msca_releaser_t *data_releaser,
                          struct ArrowArray *array) {
  msca_result_t result = MSCA_ERR;
  // Private data
  msca_prim_priv_t *priv = (msca_prim_priv_t *)malloc(sizeof(*priv));
  if (!priv) {
    result = MSCA_NOMEM;
    goto finally;
  }
  *priv = (msca_prim_priv_t){0};
  priv->data_releaser = data_releaser;
  priv->nulls_releaser = nulls_releaser;
  priv->buffers[0] = nulls;
  priv->buffers[1] = data;
  // Populate array
  *array = (struct ArrowArray){
      .length = length,
      .null_count = nulls == NULL ? 0 : -1,
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