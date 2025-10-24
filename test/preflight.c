#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <uv.h>

#include "../include/appling.h"

uv_loop_t *loop;

appling_platform_t platform;

appling_resolve_t req;

static void
on_resolve(appling_resolve_t *req, int status) {
  int err;

  assert(status == 0);

  appling_link_t link = {
    .id = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
  };

  err = appling_preflight(&platform, &link);
  assert(err == 0);
}

int
main() {
  int err;

  loop = uv_default_loop();

  err = appling_resolve(loop, &req, "test/fixtures/platform", &platform, on_resolve);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  return 0;
}
