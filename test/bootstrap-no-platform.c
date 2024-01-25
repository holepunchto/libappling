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

  appling_dkey_t dkey = {0xed, 0x17, 0xbe, 0x9d, 0xb6, 0xff, 0x78, 0x88, 0xed, 0xf7, 0x39, 0x57, 0xa7, 0x9f, 0x58, 0x9c, 0x7a, 0x9d, 0xff, 0x55, 0xfe, 0xb4, 0x32, 0xb3, 0x59, 0xed, 0xdf, 0x58, 0xe0, 0x38, 0x30, 0xca};

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
