#ifndef MSC_ERROR_H_
#define MSC_ERROR_H_

#include <stdint.h>

typedef enum : uint32_t {
  MSC_OK = 0,
  MSC_ERR = 1,
  MSC_NOMEM,
  MSC_BADARGS,
} msc_err_t;

#endif