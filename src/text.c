#include "text.h"

#include <string.h>

msc_err_t msc_text_from_c_str(const char* str, msc_text_t* o_text,
                              const msc_allocator_t* alloc) {
  msc_err_t err = MSC_ERR;
  ptrdiff_t len = (ptrdiff_t)strlen(str);
  char* data = msc_malloc(alloc, len, 1);
  if (!data) {
    err = MSC_NOMEM;
    goto err;
  }
  memcpy(data, str, len);
  err = MSC_OK;
  *o_text = (msc_text_t){
      .len = len,
      .data = data,
  };
  goto out;
err:
  if (!data) {
    msc_free(alloc, data, len, 1);
  }
out:
  return err;
}

msc_err_t msc_text_from_c_str_n(const char* str, msc_text_t* o_text,
                                ptrdiff_t maxlen,
                                const msc_allocator_t* alloc) {
  msc_err_t err = MSC_ERR;
  if (maxlen < 0) {
    err = MSC_BADARGS;
    goto err_args;
  }
  const char* end = memchr(str, '\0', (size_t)maxlen);
  ptrdiff_t len = end - str;
  char* data = msc_malloc(alloc, len, 1);
  if (!data) {
    err = MSC_NOMEM;
    goto err_alloc;
  }
  memcpy(data, str, len);
  err = MSC_OK;
  *o_text = (msc_text_t){
      .len = len,
      .data = data,
  };
  goto out;
err_alloc:
  msc_free(alloc, data, len, 1);
err_args:
out:
  return err;
}

msc_err_t msc_text_to_c_str(msc_text_t text, char** o_str,
                            const msc_allocator_t* alloc) {
  msc_err_t err = MSC_ERR;
  ptrdiff_t str_len = text.len + 1;
  char* str = msc_malloc(alloc, str_len, 1);
  if (!str) {
    err = MSC_NOMEM;
    goto err;
  }
  memcpy(str, text.data, text.len);
  str[text.len] = '\0';
  err = MSC_OK;
  *o_str = str;
  goto out;
err:
  if (!str) {
    msc_free(alloc, str, str_len, 1);
  }
out:
  return err;
}