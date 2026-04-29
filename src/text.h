#ifndef MSC_TEXT_H_
#define MSC_TEXT_H_

#include <stddef.h>

#include "alloc.h"
#include "error.h"

typedef struct {
  ptrdiff_t len;
  char* data;
} msc_text_t;

msc_err_t msc_text_from_c_str(const char* str, msc_text_t* o_text,
                              const msc_allocator_t* alloc);

msc_err_t msc_text_from_c_str_n(const char* str, msc_text_t* o_text,
                                ptrdiff_t maxlen, const msc_allocator_t* alloc);

msc_err_t msc_text_to_c_str(msc_text_t text, char** o_str,
                            const msc_allocator_t* alloc);

#endif