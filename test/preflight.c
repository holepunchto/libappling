#include <assert.h>
#include <uv.h>

#include "../include/appling.h"

int
main() {
  int err;

  appling_link_t link = {
    .id = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
  };

  err = appling_preflight("test/fixtures/platform/by-dkey/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/0", &link);
  assert(err == 0);

  return 0;
}
