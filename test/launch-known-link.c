#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <uv.h>

#include "../include/appling.h"
#include "fixtures/app.h"

uv_loop_t *loop;

appling_resolve_t req;

static void
on_resolve (appling_resolve_t *req, int status, const appling_platform_t *platform) {
  appling_link_t link = {
    .key = {0x4b, 0x32, 0x78, 0xfc, 0x44, 0xe9, 0x71, 0x6c, 0x03, 0x42, 0x71, 0x5f, 0x42, 0xe3, 0x14, 0x05, 0x0a, 0x3c, 0x82, 0x5a, 0x51, 0x05, 0x6a, 0xc5, 0x3e, 0xe8, 0x17, 0x09, 0x86, 0xa8, 0xbb, 0x86},
  };

  int err = appling_launch(loop, platform, NULL, &link);
  assert(err == 0);
}

int
main () {
  loop = uv_default_loop();

  int err = appling_resolve(loop, &req, "test/fixtures/platform", on_resolve);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  return 0;
}
