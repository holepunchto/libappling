#include <assert.h>
#include <hex.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

uv_loop_t *loop;

appling_resolve_t req;

bool resolve_called = false;

static void
on_resolve (appling_resolve_t *req, int status, const appling_platform_t *platform) {
  resolve_called = true;

  assert(status == 0);

  printf("path=%s\n", platform->path);
}

int
main () {
  loop = uv_default_loop();

  int err = appling_resolve(loop, &req, "test/fixtures/resolve/both", on_resolve);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  assert(resolve_called);

  return 0;
}
