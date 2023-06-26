#include <assert.h>
#include <hex.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

uv_loop_t *loop;

appling_resolve_t req;

bool resolve_called = false;

static void
on_resolve (appling_resolve_t *req, int status, const appling_platform_t *platform) {
  resolve_called = true;

  assert(status == 0);

  assert(platform->fork == 0);
  assert(platform->len == 1234);
  assert(memcmp(platform->key, (appling_key_t){0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa}, APPLING_KEY_LEN) == 0);

  char key[65];
  size_t key_len = 65;

  hex_encode(platform->key, APPLING_KEY_LEN, (utf8_t *) key, &key_len);

  printf(
    "exe=%s\nfork=%i\nlen=%i\nkey=%s\n",
    platform->exe,
    platform->fork,
    platform->len,
    key
  );
}

int
main () {
  loop = uv_default_loop();

  int err = appling_resolve(loop, &req, "test/fixtures", on_resolve);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  assert(resolve_called);

  return 0;
}
