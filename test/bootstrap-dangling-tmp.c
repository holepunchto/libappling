
#include <assert.h>
#include <fs.h>
#include <js.h>
#include <stdbool.h>
#include <uv.h>

#include "../include/appling.h"
#include "fixtures/app.h"

#define EXE "test/fixtures/app/" APPLING_TARGET "/" APPLING_TEST_EXE

#define DANGLING

uv_loop_t *loop;

fs_mkdir_t mkdir_req;
fs_unlink_t unlink_req;
fs_access_t access_req;

appling_bootstrap_t bootstrap_req;

js_platform_t *js;

bool bootstrap_called = false;

static void
on_access (fs_access_t *req, int status) {
  int e;

  assert(status != 0);

  e = js_destroy_platform(js);
  assert(e == 0);
}

static void
on_bootstrap (appling_bootstrap_t *req, int status) {
  int e;

  bootstrap_called = true;

  assert(status == 0);

  e = fs_access(loop, &access_req, "test/fixtures/bootstrap/dangling-tmp/by-dkey/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/0/dangling", 0, on_access);
  assert(e == 0);
}

static void
on_mkdir (fs_mkdir_t *req, int status) {
  int e;

  assert(status == 0);

  appling_dkey_t dkey = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};

  e = appling_bootstrap(loop, js, &bootstrap_req, dkey, EXE, "test/fixtures/bootstrap/dangling-tmp", on_bootstrap);
  assert(e == 0);
}

static void
on_unlink (fs_unlink_t *req, int status) {
  int e;

  (void) status;

  e = fs_mkdir(loop, &mkdir_req, "test/fixtures/bootstrap/dangling-tmp/by-dkey/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/tmp/dangling", 0777, true, on_mkdir);
  assert(e == 0);
}

int
main () {
  int e;

  loop = uv_default_loop();

  e = js_create_platform(loop, NULL, &js);
  assert(e == 0);

  e = fs_unlink(loop, &unlink_req, "test/fixtures/bootstrap/dangling-tmp/current", on_unlink);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);

  assert(bootstrap_called);

  return 0;
}
