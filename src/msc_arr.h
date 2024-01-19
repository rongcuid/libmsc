#ifndef MSC_ARR_H_
#define MSC_ARR_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/****** Basic Definitions ******/

struct ArrowSchema;
struct ArrowArray;
struct ArrowArrayStream;

/**
 * @brief The primary error type
 */
typedef enum msca_result {
  MSCARR_OK = 0,
  MSCARR_ERR,
  MSCARR_NOMEM,
  MSCARR_BADARGS,
} msca_result_t;

/****** Schema ******/
msca_result_t msca_schprim(struct ArrowSchema *schema, const char *format,
                           const char *name, const char *metadata,
                           int64_t flags);

// TODO add a vaarg schema builder?

/****** Buffers ******/

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

typedef struct msca_buf {
  void *data;
  msca_release_f release;
  void *release_context;
} msca_buf_t;

msca_result_t msca_malloc(msca_buf_t *buf, size_t size);
void msca_release(msca_buf_t *buf);

/****** Arrays ******/

void msca_arrnull(struct ArrowArray *array, size_t length);
msca_result_t msca_arrprim(struct ArrowArray *array, size_t length,
                           const msca_buf_t *validity, const msca_buf_t *data);

#endif // MSC_ARR_H_

/****** Following is the implementation ******/

#ifdef MSC_ARR_IMPLEMENTATION

#include <string.h>

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

static void msca_schprim_release(struct ArrowSchema *schema) {
  schema->release = NULL;
  free((void *)schema->name);
  free((void *)schema->format);
  free((void *)schema->metadata);
}

msca_result_t msca_schprim(struct ArrowSchema *schema, const char *format,
                           const char *name, const char *metadata,
                           int64_t flags) {
  msca_result_t result = MSCARR_ERR;
  if (format == NULL) {
    result = MSCARR_BADARGS;
    goto finally;
  }
  size_t format_len = strnlen(format, 1024);
  if (format_len == 0) {
    result = MSCARR_BADARGS;
    goto finally;
  }
  char *tmp_format = NULL, *tmp_name = NULL, *tmp_metadata = NULL;
  if (!strchr("nbcCsSiIlLefgzZuUdwt", format[0])) {
    result = MSCARR_BADARGS;
    goto finally;
  }
  // Format
  // TODO: check format string validity
  tmp_format = malloc(format_len + 1);
  if (!tmp_format) {
    result = MSCARR_NOMEM;
    goto finally;
  }
  memcpy(tmp_format, format, format_len + 1);
  // Name
  if (name) {
    size_t name_len = strnlen(name, 1024);
    tmp_name = malloc(name_len + 1);
    if (!tmp_name) {
      result = MSCARR_NOMEM;
      goto err_after_fmt;
    }
    memcpy(tmp_name, name, name_len + 1);
  }
  // Meta
  if (metadata) {
    size_t meta_len = strnlen(metadata, 1024);
    tmp_metadata = malloc(meta_len + 1);
    if (!tmp_metadata) {
      result = MSCARR_NOMEM;
      goto err_after_name;
    }
    memcpy(tmp_metadata, metadata, meta_len + 1);
  }
  *schema = (struct ArrowSchema){
      .format = tmp_format,
      .name = tmp_name,
      .metadata = tmp_metadata,
      .release = &msca_schprim_release,
      .flags = flags,
  };
  goto ok;
err_after_name:
  free(tmp_name);
err_after_fmt:
  free(tmp_format);
ok:
  result = MSCARR_OK;
finally:
  return result;
}

msca_result_t msca_malloc(msca_buf_t *buf, size_t size) {
  void *data = malloc(size);
  if (!data) {
    return MSCARR_NOMEM;
  }
  *buf = (msca_buf_t){
      .data = data,
      .release = msca_malloc_release,
      .release_context = NULL,
  };
  return MSCARR_OK;
}

void msca_release(msca_buf_t *buf) {
  if (buf->release) {
    buf->release(buf->release_context, buf->data);
  }
}

static void msca_malloc_release_(void *context, void *data) { free(data); }

msca_release_f msca_malloc_release = &msca_malloc_release_;

static void msca_release_null(struct ArrowArray *arr) { arr->release = NULL; }

void msca_arrnull(struct ArrowArray *array, size_t length) {
  *array = (struct ArrowArray){0};
  array->length = length;
  array->null_count = length;
  array->release = msca_release_null;
}

struct msca_arrprim_priv {
  msca_buf_t validity;
  msca_buf_t data;
  const void *buffers[2];
};

static void msca_release_prim(struct ArrowArray *arr) {
  struct msca_arrprim_priv *priv =
      (struct msca_arrprim_priv *)arr->private_data;
  msca_release(&priv->validity);
  msca_release(&priv->data);
  free(priv);
  arr->release = NULL;
}

msca_result_t msca_arrprim(struct ArrowArray *array, size_t length,
                           const msca_buf_t *validity, const msca_buf_t *data) {
  msca_result_t result = MSCARR_ERR;
  if (data == NULL) {
    result = MSCARR_BADARGS;
    goto finally;
  }
  // Private data
  struct msca_arrprim_priv *priv =
      (struct msca_arrprim_priv *)calloc(1, sizeof(*priv));
  if (!priv) {
    result = MSCARR_NOMEM;
    goto finally;
  }
  if (validity != NULL) {
    priv->validity = *validity;
    priv->buffers[0] = validity->data;
  }
  priv->data = *data;
  priv->buffers[1] = data->data;
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
  result = MSCARR_OK;
finally:
  return result;
}

#endif