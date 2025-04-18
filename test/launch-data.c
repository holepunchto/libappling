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
  appling_app_t app = {
    .path = EXE,
  };

  appling_link_t link = {
    .id = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
    .data = "this-is-some-data",
  };

  status = appling_launch(&platform, &app, &link, "Example");
  assert(status == 0);
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
