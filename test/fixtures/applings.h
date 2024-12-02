#ifndef APPLING_TEST_FIXTURES_APPLINGS_H
#define APPLING_TEST_FIXTURES_APPLINGS_H

#include <assert.h>
#include <compact.h>
#include <stdlib.h>
#include <utf.h>
#include <uv.h>

#include "../../include/appling.h"
#include "app.h"

#define EXE "test/fixtures/app/" APPLING_TARGET "/" APPLING_TEST_EXE
#define KEY "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

static int
on_preencode(compact_state_t *state, void *array, size_t i, void *data) {
  int err;

  appling_app_t *apps = (appling_app_t *) array;

  utf8_string_view_t path = utf8_string_view_init((utf8_t *) apps[i].path, strlen(apps[i].path));
  err = compact_preencode_utf8(state, path);
  assert(err == 0);

  utf8_string_view_t key = utf8_string_view_init((utf8_t *) apps[i].key, strlen(apps[i].key));
  err = compact_preencode_utf8(state, key);
  assert(err == 0);

  return 0;
}

static int
on_encode(compact_state_t *state, void *array, size_t i, void *data) {
  int err;

  appling_app_t *apps = (appling_app_t *) array;

  utf8_string_view_t path = utf8_string_view_init((utf8_t *) apps[i].path, strlen(apps[i].path));
  err = compact_encode_utf8(state, path);
  assert(err == 0);

  utf8_string_view_t key = utf8_string_view_init((utf8_t *) apps[i].key, strlen(apps[i].key));
  err = compact_encode_utf8(state, key);
  assert(err == 0);

  return 0;
}

static inline void
appling_generate_paths(uv_loop_t *loop) {
  int err;

  appling_app_t apps[] = {
    {
      .path = EXE,
      .key = KEY,
    },
  };

  compact_state_t state = {0, 0};

  err = compact_preencode_uint(&state, 0);
  assert(err == 0);

  err = compact_preencode_array(&state, apps, 1, NULL, on_preencode);
  assert(err == 0);

  state.buffer = malloc(state.end);

  err = compact_encode_uint(&state, 0);
  assert(err == 0);

  err = compact_encode_array(&state, apps, 1, NULL, on_encode);
  assert(err == 0);

  uv_fs_t req;

  err = uv_fs_open(loop, &req, "test/fixtures/platform/applings", O_WRONLY | O_CREAT | O_TRUNC, 0666, NULL);
  assert(err > 0);

  int fd = err;

  uv_fs_req_cleanup(&req);

  uv_buf_t buf = uv_buf_init((char *) state.buffer, state.end);

  err = uv_fs_write(loop, &req, fd, &buf, 1, 0, NULL);
  assert(err > 0);

  uv_fs_req_cleanup(&req);

  err = uv_fs_close(loop, &req, fd, NULL);
  assert(err == 0);

  uv_fs_req_cleanup(&req);
}

#endif // APPLING_TEST_FIXTURES_APPLINGS_H
