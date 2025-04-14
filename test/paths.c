#include <assert.h>
#include <stdio.h>
#include <uv.h>

#include "../include/appling.h"

uv_loop_t *loop;

appling_paths_t req;

bool paths_called = false;

static void
on_paths(appling_paths_t *req, int status, const appling_app_t *apps, size_t len) {
  paths_called = true;

  assert(status == 0);

  for (size_t i = 0; i < len; i++) {
    const appling_app_t *app = &apps[i];

    printf("path=%s\n", app->path);
  }
}

int
main() {
  loop = uv_default_loop();

  int err = appling_paths(loop, &req, "test/fixtures/platform", on_paths);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  assert(paths_called);

  return 0;
}
