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

  appling_key_t key = {0x6b, 0x83, 0x74, 0xf1, 0xc0, 0x80, 0x9e, 0xd2, 0x3c, 0xfc, 0x37, 0x1e, 0x87, 0x89, 0x6c, 0x8d, 0x3b, 0xb5, 0x93, 0xf2, 0x45, 0x1d, 0x4d, 0x8d, 0xe8, 0x95, 0xd6, 0x28, 0x94, 0x18, 0x18, 0xdc};
  const char *link = "pear://keet";

  e = appling_bootstrap(loop, js, &bootstrap_req, key, "test/fixtures/bootstrap/no-platform", link, on_bootstrap);
  assert(e == 0);
}

int
main() {
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
