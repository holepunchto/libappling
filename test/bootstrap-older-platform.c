#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <uv.h>

#include "../include/holepunch.h"

#define EXE "test/fixtures/app/darwin-arm64/Example.app/Contents/MacOS/Example"

uv_loop_t *loop;

holepunch_bootstrap_t req;

bool bootstrap_called = false;

static void
on_bootstrap (holepunch_bootstrap_t *req, int status, const holepunch_app_t *app) {
  bootstrap_called = true;

  assert(status == 0);

  assert(app->platform.len == 1234);
}

int
main () {
  loop = uv_default_loop();

  holepunch_platform_t platform = {
    .exe = "",
    .key = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
    .fork = 0,
    .len = 1234 - 1,
  };

  int err = holepunch_bootstrap(loop, &req, EXE, "test/fixtures/bootstrap/older-platform", &platform, on_bootstrap);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  assert(bootstrap_called);
}
