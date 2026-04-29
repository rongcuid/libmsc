#ifndef MSCC_ERROR_H_
#define MSCC_ERROR_H_

#include <stdint.h>

typedef enum : uint32_t {
  MSCC_OK = 0,
  MSCC_ERR = 1,
  MSCC_NOMEM,
  MSCC_BADARGS,
} mscc_err_t;

#endif