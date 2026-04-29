#include "text.h"

#include <string.h>

mscc_err_t mscc_text_from_c_str(const char *str, mscc_text_t *o_text,
                                const mscc_allocator_t *alloc) {
  mscc_err_t err = MSCC_ERR;
  ptrdiff_t len = (ptrdiff_t)strlen(str);
  char *data = mscc_malloc(alloc, len, 1);
  if (!data) {
    err = MSCC_NOMEM;
    goto err;
  }
  memcpy(data, str, len);
  err = MSCC_OK;
  *o_text = (mscc_text_t){
      .len = len,
      .data = data,
  };
  goto out;
err:
  if (!data) {
    mscc_free(alloc, data, len, 1);
  }
out:
  return err;
}

mscc_err_t mscc_text_from_c_str_n(const char *str, mscc_text_t *o_text,
                                  ptrdiff_t maxlen,
                                  const mscc_allocator_t *alloc) {
  mscc_err_t err = MSCC_ERR;
  if (maxlen < 0) {
    err = MSCC_BADARGS;
    goto err_args;
  }
  ptrdiff_t len = (ptrdiff_t)strnlen(str, (size_t)maxlen);
  char *data = mscc_malloc(alloc, len, 1);
  if (!data) {
    err = MSCC_NOMEM;
    goto err_alloc;
  }
  memcpy(data, str, len);
  err = MSCC_OK;
  *o_text = (mscc_text_t){
      .len = len,
      .data = data,
  };
  goto out;
err_alloc:
  mscc_free(alloc, data, len, 1);
err_args:
out:
  return err;
}

mscc_err_t mscc_text_to_c_str(mscc_text_t text, char **o_str,
                              const mscc_allocator_t *alloc) {
  mscc_err_t err = MSCC_ERR;
  ptrdiff_t str_len = text.len + 1;
  char *str = mscc_malloc(alloc, str_len, 1);
  if (!str) {
    err = MSCC_NOMEM;
    goto err;
  }
  memcpy(str, text.data, text.len);
  str[text.len] = '\0';
  err = MSCC_OK;
  *o_str = str;
  goto out;
err:
  if (!str) {
    mscc_free(alloc, str, str_len, 1);
  }
out:
  return err;
}