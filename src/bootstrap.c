#include <assert.h>
#include <bare.h>
#include <hex.h>
#include <js.h>
#include <path.h>
#include <string.h>
#include <uv.h>

#include "../include/appling.h"
#include "bootstrap.bundle.h"

static void
on_thread (void *data) {
  int err;

  appling_bootstrap_t *req = (appling_bootstrap_t *) data;

  uv_loop_t loop;
  err = uv_loop_init(&loop);
  assert(err == 0);

  char dkey[65];
  size_t dkey_len = 65;

  err = hex_encode(req->dkey, APPLING_DKEY_LEN, (utf8_t *) dkey, &dkey_len);
  assert(err == 0);

  char *argv[2] = {dkey, req->dir};

  bare_t *bare;
  err = bare_setup(&loop, req->js, NULL, 2, argv, NULL, &bare);
  assert(err == 0);

  uv_buf_t source = uv_buf_init((char *) bundle, bundle_len);

  err = bare_run(bare, "/bootstrap.bundle", &source);
  assert(err == 0);

  err = bare_teardown(bare, &req->status);
  assert(err == 0);

  err = uv_loop_close(&loop);
  assert(err == 0);

  err = uv_async_send(&req->signal);
  assert(err == 0);
}

static void
on_close (uv_handle_t *handle) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) handle->data;

  if (req->cb) req->cb(req, req->status);
}

static void
on_signal (uv_async_t *handle) {
  uv_close((uv_handle_t *) handle, on_close);
}

int
appling_bootstrap (uv_loop_t *loop, js_platform_t *js, appling_bootstrap_t *req, const appling_dkey_t dkey, const char *dir, appling_bootstrap_cb cb) {
  int err;

  req->loop = loop;
  req->js = js;
  req->cb = cb;
  req->status = 0;
  req->signal.data = (void *) req;

  err = uv_async_init(loop, &req->signal, on_signal);
  if (err < 0) return err;

  memcpy(req->dkey, dkey, sizeof(appling_dkey_t));

  if (dir && path_is_absolute(dir, path_behavior_system)) strcpy(req->dir, dir);
  else if (dir) {
    appling_path_t cwd;
    size_t path_len = sizeof(appling_path_t);

    int err = uv_cwd(cwd, &path_len);
    if (err < 0) return err;

    path_len = sizeof(appling_path_t);

    path_join(
      (const char *[]){cwd, dir, NULL},
      req->dir,
      &path_len,
      path_behavior_system
    );
  } else {
    appling_path_t homedir;
    size_t path_len = sizeof(appling_path_t);

    int err = uv_os_homedir(homedir, &path_len);
    if (err < 0) return err;

    path_len = sizeof(appling_path_t);

    path_join(
      (const char *[]){homedir, appling_platform_dir, NULL},
      req->dir,
      &path_len,
      path_behavior_system
    );
  }

  return uv_thread_create(&req->thread, on_thread, (void *) req);
}
