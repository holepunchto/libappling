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
on_bootstrap(appling_bootstrap_t *req, int status) {
  int e;

  bootstrap_called = true;

  assert(status == 0);

  e = js_destroy_platform(js);
  assert(e == 0);
}

static void
on_unlink(fs_unlink_t *req, int status) {
  int e;

  (void) status;

  appling_key_t key = {0x6d, 0xd8, 0x97, 0x2d, 0xb0, 0x87, 0xad, 0x75, 0x41, 0x9a, 0x0b, 0x55, 0x4f, 0x6e, 0xa1, 0xfb, 0x22, 0x22, 0x3b, 0xa1, 0xf2, 0xc4, 0x84, 0x54, 0x41, 0xe0, 0x78, 0x8a, 0xf3, 0x0e, 0xf3, 0x7d};

  e = appling_bootstrap(loop, js, &bootstrap_req, key, "test/fixtures/bootstrap/no-platform-v2", on_bootstrap);
  assert(e == 0);
}

int
main() {
  int e;

  loop = uv_default_loop();

  e = js_create_platform(loop, NULL, &js);
  assert(e == 0);

  e = fs_unlink(loop, &unlink_req, "test/fixtures/bootstrap/no-platform-v2/current", on_unlink);
  assert(e == 0);

  e = uv_run(loop, UV_RUN_DEFAULT);
  assert(e == 0);

  assert(bootstrap_called);

  return 0;
}
