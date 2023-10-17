#include <assert.h>
#include <fs.h>
#include <stdbool.h>
#include <stdio.h>
#include <uv.h>

#include "../include/appling.h"
#include "fixtures/app.h"

#define EXE "test/fixtures/app/" APPLING_TARGET "/" APPLING_TEST_EXE

uv_loop_t *loop;

fs_unlink_t unlink_req;

appling_bootstrap_t bootstrap_req;

bool bootstrap_called = false;

static void
on_bootstrap (appling_bootstrap_t *req, int status) {
  bootstrap_called = true;

  assert(status == 0);
}

static void
on_unlink (fs_unlink_t *req, int status) {
  appling_dkey_t dkey = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};

  int err = appling_bootstrap(loop, &bootstrap_req, dkey, EXE, "test/fixtures/bootstrap/existing-corestores", on_bootstrap);
  assert(err == 0);
}

int
main () {
  loop = uv_default_loop();

  fs_unlink(loop, &unlink_req, "test/fixtures/bootstrap/existing-corestores/current", on_unlink);

  uv_run(loop, UV_RUN_DEFAULT);

  assert(bootstrap_called);

  return 0;
}
