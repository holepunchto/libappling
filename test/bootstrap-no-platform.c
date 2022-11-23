#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <uv.h>

#include "../include/holepunch.h"
#include "fixtures/app.h"

#define EXE "test/fixtures/app/" HOLEPUNCH_RUNTIME "/" HOLEPUNCH_TEST_EXE

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

  int err = holepunch_bootstrap(loop, &req, EXE, "test/fixtures/bootstrap/no-platform", NULL, on_bootstrap);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  assert(bootstrap_called);
}
