#include <assert.h>
#include <hex.h>
#include <stdbool.h>
#include <stdio.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

uv_loop_t *loop;

appling_platform_t platform;

appling_resolve_t req;

bool resolve_called = false;

static void
on_resolve (appling_resolve_t *req, int status) {
  resolve_called = true;

  assert(status == 0);

  printf("path=%s\n", platform.path);
  printf("length=%lld\n", platform.length);
}

int
main () {
  int err;

  loop = uv_default_loop();

  err = appling_resolve(loop, &req, "test/fixtures/resolve/current", &platform, 123, on_resolve);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  assert(resolve_called);

  return 0;
}
