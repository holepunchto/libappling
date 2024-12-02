#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <uv.h>

#include "../include/appling.h"
#include "fixtures/app.h"

#define EXE "test/fixtures/app/" APPLING_TARGET "/" APPLING_TEST_EXE

uv_loop_t *loop;

appling_platform_t platform;

appling_resolve_t req;

static void
on_resolve(appling_resolve_t *req, int status) {
  assert(status == 0);

  appling_app_t app = {
    .path = EXE,
  };

  appling_link_t link = {
    .key = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
  };

  int err = appling_launch(&platform, &app, &link);
  assert(err == 0);
}

int
main() {
  int err;

  loop = uv_default_loop();

  err = appling_resolve(loop, &req, "test/fixtures/platform", &platform, 0, on_resolve);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  return 0;
}
