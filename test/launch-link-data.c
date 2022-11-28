#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <uv.h>

#include "../include/appling.h"
#include "fixtures/app.h"

#define EXE "test/fixtures/app/" APPLING_RUNTIME "/" APPLING_TEST_EXE

uv_loop_t *loop;

appling_resolve_t req;
appling_process_t process;

bool exit_called = false;

static void
on_process_exit (appling_process_t *process, int64_t exit_status, int term_signal) {
  exit_called = true;

  assert(exit_status == 0);
}

static void
on_resolve (appling_resolve_t *req, int status, const appling_platform_t *platform) {
  appling_link_t link = {
    .key = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
    .data = "this-is-some-data",
  };

  appling_app_t app = {
    .platform = *platform,
    .exe = EXE,
    .key = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
  };

  int err = appling_launch(loop, &process, &link, &app, on_process_exit);
  assert(err == 0);
}

int
main () {
  loop = uv_default_loop();

  int err = appling_resolve(loop, &req, "test/fixtures", on_resolve);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  assert(exit_called);

  return 0;
}
