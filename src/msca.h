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

typedef enum msca_result {
  MSCA_OK = 0,
  MSCA_ERR,
  MSCA_NOMEM,
} msca_result;

void msca_mknull(size_t length, struct ArrowArray *array);

msca_result msca_mkprim(void *data, size_t length,
                        void (*data_release)(void *data),
                        struct ArrowArray *array);

#endif // MSCA_H__

#ifdef MSCA_IMPLEMENTATION

static void msca_release_null(struct ArrowArray *arr) { arr->release = NULL; }

void msca_mknull(size_t length, struct ArrowArray *array) {
  *array = (struct ArrowArray){0};
  array->length = length;
  array->null_count = length;
  array->release = msca_release_null;
}

typedef struct msca_prim_priv {
  void (*data_release)(void *data);
  const void *buffers[2];
} msca_prim_priv_t;

static void msca_release_prim(struct ArrowArray *arr) {
  msca_prim_priv_t *priv = (msca_prim_priv_t *)arr->private_data;
  if (priv->data_release) {
    priv->data_release((void *)arr->buffers[1]);
  }
  free(priv);
  arr->release = NULL;
}

msca_result msca_mkprim(void *data, size_t length,
                        void (*data_release)(void *data),
                        struct ArrowArray *array) {
  msca_result result = MSCA_ERR;
  msca_prim_priv_t *priv = (msca_prim_priv_t *)malloc(sizeof(*priv));
  if (!priv) {
    result = MSCA_NOMEM;
    goto finally;
  }
  *priv = (msca_prim_priv_t){0};
  priv->data_release = data_release;
  priv->buffers[1] = data;
  *array = (struct ArrowArray){0};
  array->length = length;
  array->null_count = 0;
  array->n_buffers = 2;
  array->buffers = priv->buffers;
  array->release = msca_release_prim;
  array->private_data = priv;
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