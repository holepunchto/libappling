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
  if (req->error) fprintf(stderr, "on_bootstrap: %s\n\n", req->error);
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

  appling_key_t key = {0x1f, 0x1a, 0xcd, 0x89, 0xab, 0x72, 0xb0, 0x2b, 0x3f, 0x27, 0x9f, 0x24, 0x7e, 0x66, 0x74, 0x68, 0xdb, 0xe3, 0x7e, 0x6a, 0x85, 0xc8, 0xe0, 0x33, 0x55, 0x20, 0x79, 0x42, 0x55, 0x03, 0x02, 0x39};
  const char *link = "pear://ma9zo8zmfat3ih314mne1q47shrfho1odyfm9methgnn7b866w6y";

  e = appling_bootstrap(loop, js, &bootstrap_req, key, "test/fixtures/bootstrap/no-platform", on_bootstrap, link);
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
