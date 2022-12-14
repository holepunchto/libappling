#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <uv.h>

#include "../include/appling.h"
#include "fixtures/app.h"

#define EXE "test/fixtures/app/" APPLING_RUNTIME "/" APPLING_TEST_EXE

uv_loop_t *loop;

appling_bootstrap_t req;

bool bootstrap_called = false;

static void
on_bootstrap (appling_bootstrap_t *req, int status, const appling_app_t *app) {
  bootstrap_called = true;

  assert(status == 0);

  assert(app->platform.len == 1234 + 1);
}

int
main () {
  loop = uv_default_loop();

  appling_platform_t platform = {
    .exe = "",
    .key = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa},
    .fork = 0,
    .len = 1234 + 1,
  };

  int err = appling_bootstrap(loop, &req, EXE, "test/fixtures/bootstrap/newer-platform", &platform, on_bootstrap);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  assert(bootstrap_called);

  return 0;
}
