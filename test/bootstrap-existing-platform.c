#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <uv.h>

#include "../include/appling.h"
#include "fixtures/app.h"

#define EXE "test/fixtures/app/" APPLING_TARGET "/" APPLING_TEST_EXE

uv_loop_t *loop;

appling_resolve_t resolve;
appling_bootstrap_t bootstrap;

bool bootstrap_called = false;

static void
on_bootstrap (appling_bootstrap_t *req, int status, const appling_platform_t *platform, const appling_app_t *app) {
  bootstrap_called = true;

  assert(status == 0);
}

static void
on_resolve (appling_resolve_t *req, int status, const appling_platform_t *platform) {
  assert(status == 0);

  printf("exe=%s\n", platform->path);

  appling_key_t key = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};

  int err = appling_bootstrap(loop, &bootstrap, key, EXE, "test/fixtures/bootstrap/existing-platform", platform, on_bootstrap);
  assert(err == 0);
}

int
main () {
  loop = uv_default_loop();

  appling_platform_t platform = {
    .path = "",
  };

  int err = appling_resolve(loop, &resolve, "test/fixtures/platform", on_resolve);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  assert(bootstrap_called);

  return 0;
}
