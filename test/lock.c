#include <assert.h>
#include <stdbool.h>
#include <uv.h>

#include "../include/appling.h"

uv_loop_t *loop;

appling_lock_t req;

bool lock_called = false;

static void
on_lock(appling_lock_t *req, int status) {
  lock_called = true;

  assert(status == 0);
}

int
main() {
  loop = uv_default_loop();

  int err = appling_lock(loop, &req, "test/fixtures/lock", on_lock);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  assert(lock_called);

  return 0;
}
