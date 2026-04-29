#ifndef MSCC_TEXT_H_
#define MSCC_TEXT_H_

#include <stddef.h>

#include "alloc.h"
#include "error.h"

typedef struct {
  ptrdiff_t len;
  char *data;
} mscc_text_t;

mscc_err_t mscc_text_from_c_str(const char *str, mscc_text_t *o_text,
                                const mscc_allocator_t *alloc);

mscc_err_t mscc_text_from_c_str_n(const char *str, mscc_text_t *o_text,
                                  ptrdiff_t maxlen,
                                  const mscc_allocator_t *alloc);

mscc_err_t mscc_text_to_c_str(mscc_text_t text, char **o_str,
                              const mscc_allocator_t *alloc);

#endif