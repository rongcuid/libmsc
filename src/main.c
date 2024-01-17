#include <assert.h>
#include <stdio.h>

#define MSCA_IMPLEMENTATION
#include "msca.h"

void print_arr(const struct ArrowArray *arr) {
  printf("length: %lld, null_count: %lld, offset: %lld, n_buffers: "
         "%lld, n_children: %lld\n",
         arr->length, arr->null_count, arr->offset, arr->n_buffers,
         arr->n_children);
}

int main(int argc, char **argv) {
  printf("Create null array!\n");
  struct ArrowArray nullarr;
  msca_mknull(1000, &nullarr);
  assert(nullarr.release != NULL);
  print_arr(&nullarr);
  nullarr.release(&nullarr);
  assert(nullarr.release == NULL);
  return 0;
}
