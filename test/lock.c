#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <uv.h>

#include "../include/holepunch.h"

uv_loop_t *loop;

holepunch_lock_t req;

bool lock_called = false;

static void
on_lock (holepunch_lock_t *req, int status) {
  lock_called = true;

  assert(status == 0);
}

int
main () {
  loop = uv_default_loop();

  int err = holepunch_lock(loop, &req, "test/fixtures", on_lock);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  assert(lock_called);

  return 0;
}
