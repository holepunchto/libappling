#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <uv.h>

#include "../include/holepunch.h"

uv_loop_t *loop;

holepunch_extract_t req;

bool extract_called = false;

static void
on_extract (holepunch_extract_t *req, int status) {
  extract_called = true;

  assert(status == 0);

  {
    uv_fs_t req;

    int err = uv_fs_stat(loop, &req, "test/fixtures/extract/archive/hello.txt", NULL);
    assert(err == 0);
  }
}

int
main () {
  loop = uv_default_loop();

  int err = holepunch_extract(loop, &req, "test/fixtures/extract/archive.tar", "test/fixtures/extract/archive", on_extract);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  assert(extract_called);
}
