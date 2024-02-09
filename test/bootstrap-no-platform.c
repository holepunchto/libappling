#include <assert.h>
#include <fs.h>
#include <js.h>
#include <stdbool.h>
#include <stdio.h>
#include <uv.h>

#include "../include/appling.h"

uv_loop_t *loop;

fs_unlink_t unlink_req;

appling_bootstrap_t bootstrap_req;

js_platform_t *js;

bool bootstrap_called = false;

static void
on_bootstrap (appling_bootstrap_t *req, int status) {
  int e;

  bootstrap_called = true;

  assert(status == 0);

  e = js_destroy_platform(js);
  assert(e == 0);
}

static void
on_unlink (fs_unlink_t *req, int status) {
  int e;

  (void) status;

  appling_dkey_t dkey = {0x93, 0xfd, 0x0e, 0x8e, 0xaa, 0xf1, 0xbb, 0xc1, 0x71, 0xff, 0x7b, 0xed, 0x96, 0x84, 0x77, 0x64, 0xef, 0x91, 0xdb, 0xd4, 0x05, 0x3a, 0x0d, 0x0a, 0x29, 0x41, 0xfd, 0x58, 0x45, 0x09, 0x71, 0x5c};

  e = appling_bootstrap(loop, js, &bootstrap_req, dkey, "test/fixtures/bootstrap/no-platform", on_bootstrap);
  assert(e == 0);
}

int
main () {
  int e;

  loop = uv_default_loop();

  e = js_create_platform(loop, NULL, &js);
  assert(e == 0);

  e = fs_unlink(loop, &unlink_req, "test/fixtures/bootstrap/no-platform/current", on_unlink);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);

  assert(bootstrap_called);

  return 0;
}
