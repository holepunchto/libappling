
#include <assert.h>
#include <fs.h>
#include <stdbool.h>
#include <stdio.h>
#include <uv.h>

#include "../include/appling.h"
#include "fixtures/app.h"

#define EXE "test/fixtures/app/" APPLING_RUNTIME "/" APPLING_TEST_EXE

#define DANGLING

uv_loop_t *loop;

fs_mkdir_t mkdir_req;
fs_unlink_t unlink_req;
fs_access_t access_req;

appling_bootstrap_t bootstrap_req;

bool bootstrap_called = false;

static void
on_access (fs_access_t *req, int status) {
  assert(status != 0);
}

static void
on_bootstrap (appling_bootstrap_t *req, int status, const appling_app_t *app) {
  bootstrap_called = true;

  assert(status == 0);

  fs_access(loop, &access_req, "test/fixtures/bootstrap/dangling-tmp/by-dkey/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/0/dangling", 0, on_access);
}

static void
on_mkdir (fs_mkdir_t *req, int status) {
  assert(status == 0);
  appling_key_t key = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};

  int err = appling_bootstrap(loop, &bootstrap_req, key, EXE, "test/fixtures/bootstrap/dangling-tmp", NULL, on_bootstrap);
  assert(err == 0);
}

static void
on_unlink (fs_unlink_t *req, int status) {
  fs_mkdir(loop, &mkdir_req, "test/fixtures/bootstrap/dangling-tmp/by-dkey/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/tmp/dangling", 0777, true, on_mkdir);
}

int
main () {
  loop = uv_default_loop();

  fs_unlink(loop, &unlink_req, "test/fixtures/bootstrap/dangling-tmp/current", on_unlink);

  uv_run(loop, UV_RUN_DEFAULT);

  assert(bootstrap_called);

  return 0;
}
