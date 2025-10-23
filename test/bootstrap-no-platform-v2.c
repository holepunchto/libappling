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

  appling_key_t key = {0xbf, 0x47, 0x95, 0x07, 0x28, 0xa6, 0xa2, 0x24, 0xa9, 0xcf, 0x62, 0xa5, 0xec, 0x36, 0xf0, 0x9f, 0x55, 0x2e, 0xe8, 0xf7, 0x27, 0xf1, 0xcd, 0x6a, 0x69, 0x58, 0x44, 0x26, 0xde, 0xe6, 0xfd, 0x03};

  appling_link_t link;
  e = appling_parse("pear://runtime", &link);
  assert(e == 0);

  e = appling_bootstrap(loop, js, &bootstrap_req, key, &link, "test/fixtures/bootstrap/no-platform-v2", on_bootstrap);
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
