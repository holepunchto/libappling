#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <uv.h>

#include "../include/appling.h"
#include "fixtures/app.h"

#define EXE "test/fixtures/app/" APPLING_TARGET "/" APPLING_TEST_EXE

uv_loop_t *loop;

appling_resolve_t req;

static void
on_resolve (appling_resolve_t *req, int status, const appling_platform_t *platform) {
  appling_app_t app = {
    .path = EXE,
  };

  appling_link_t link = {
    .key = {0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb},
    .data = "this-is-some-data",
  };

  int err = appling_launch(loop, platform, &app, &link);
  assert(err == 0);
}

int
main () {
  loop = uv_default_loop();

  int err = appling_resolve(loop, &req, "test/fixtures/platform", on_resolve);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  return 0;
}
