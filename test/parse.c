#include <assert.h>
#include <string.h>

#include "../include/appling.h"

int
main () {
  appling_link_t link;

  appling_parse("punch://aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/data", &link);
  assert(strcmp(link.key, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") == 0);
  assert(strcmp(link.data, "data") == 0);

  return 0;
}
