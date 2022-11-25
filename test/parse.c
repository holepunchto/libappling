#include <assert.h>
#include <string.h>

#include "../include/holepunch.h"

int
main () {
  holepunch_link_t link;

  holepunch_parse("punch://aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/data", &link);
  assert(strcmp(link.key, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") == 0);
  assert(strcmp(link.data, "data") == 0);

  return 0;
}
