#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <uv.h>

#include "../include/appling.h"
#include "fixtures/app.h"

uv_loop_t *loop;

appling_resolve_t req;

static void
on_resolve (appling_resolve_t *req, int status, const appling_platform_t *platform) {
  appling_app_t app = {
#if defined(APPLING_OS_DARWIN)
    .path = "/Applications/Keet.app/Contents/MacOS/Keet",
#endif
  };

  int err = appling_launch(loop, platform, &app, NULL);
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
