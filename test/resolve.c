#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <uv.h>

#include "../include/holepunch.h"

uv_loop_t *loop;

holepunch_resolve_t req;

bool resolve_called = false;

static void
on_resolve (holepunch_resolve_t *req, int status, const holepunch_platform_t *platform) {
  resolve_called = true;

  assert(status == 0);

  assert(platform->fork == 0);
  assert(platform->len == 1234);
  assert(strcmp(platform->key, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") == 0);

  printf(
    "exe=%s\nfork=%i\nlen=%i\nkey=%s\n",
    platform->exe,
    platform->fork,
    platform->len,
    platform->key
  );
}

int
main () {
  loop = uv_default_loop();

  int err = holepunch_resolve(loop, &req, "test/fixtures", on_resolve);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  assert(resolve_called);

  return 0;
}
