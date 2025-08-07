#include <assert.h>
#include <bare.h>
#include <js.h>
#include <path.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"
#include "bootstrap.bundle.h"

static js_value_t *
appling_bootstrap__error(js_env_t *env, js_callback_info_t *info) {
  int err;

  appling_bootstrap_t *req;

  size_t argc = 1;
  js_value_t *argv[1];

  err = js_get_callback_info(env, info, &argc, argv, NULL, (void **) &req);
  assert(err == 0);

  assert(argc == 1);

  size_t len;
  err = js_get_value_string_utf8(env, argv[0], NULL, 0, &len);
  assert(err == 0);

  len += 1 /* NULL */;

  req->error = calloc(len, sizeof(char));

  err = js_get_value_string_utf8(env, argv[0], (utf8_t *) req->error, len, NULL);
  assert(err == 0);

  return NULL;
}

static void
appling_bootstrap__on_thread(void *data) {
  int err;

  appling_bootstrap_t *req = (appling_bootstrap_t *) data;

  uv_loop_t loop;
  err = uv_loop_init(&loop);
  assert(err == 0);

  js_env_t *env;

  bare_t *bare;
  err = bare_setup(&loop, req->js, &env, 0, NULL, NULL, &bare);
  assert(err == 0);

  js_handle_scope_t *scope;
  err = js_open_handle_scope(env, &scope);
  assert(err == 0);

  js_value_t *global;
  err = js_get_global(env, &global);
  assert(err == 0);

  js_value_t *exports;
  err = js_create_object(env, &exports);
  assert(err == 0);

  err = js_set_named_property(env, global, "Appling", exports);
  assert(err == 0);

  void *buffer;

  js_value_t *key;
  err = js_create_arraybuffer(env, sizeof(req->key), &buffer, &key);
  assert(err == 0);

  memcpy(buffer, req->key, sizeof(req->key));

  err = js_set_named_property(env, exports, "key", key);
  assert(err == 0);

  js_value_t *directory;
  err = js_create_string_utf8(env, (utf8_t *) req->dir, -1, &directory);
  assert(err == 0);

  err = js_set_named_property(env, exports, "directory", directory);
  assert(err == 0);

  void *buffer_link;

  js_value_t *link;
  err = js_create_arraybuffer(env, sizeof(req->link.id), &buffer_link, &link);
  assert(err == 0);

  memcpy(buffer_link, req->link.id, sizeof(req->link.id));

  err = js_set_named_property(env, exports, "link", link);
  assert(err == 0);

  js_value_t *error;
  err = js_create_function(env, "error", -1, appling_bootstrap__error, (void *) req, &error);
  assert(err == 0);

  err = js_set_named_property(env, exports, "error", error);
  assert(err == 0);

  err = js_close_handle_scope(env, scope);
  assert(err == 0);

  uv_buf_t source = uv_buf_init((char *) bootstrap_bundle, bootstrap_bundle_len);

  err = bare_load(bare, "bare:/appling.bundle", &source, NULL);
  assert(err == 0);

  err = bare_run(bare);
  assert(err == 0);

  err = bare_teardown(bare, &req->status);
  assert(err == 0);

  err = uv_loop_close(&loop);
  assert(err == 0);

  err = uv_async_send(&req->signal);
  assert(err == 0);
}

static void
appling_bootstrap__on_close(uv_handle_t *handle) {
  appling_bootstrap_t *req = (appling_bootstrap_t *) handle->data;

  if (req->cb) req->cb(req, req->status);

  if (req->error) free(req->error);
}

static void
appling_bootstrap__on_signal(uv_async_t *handle) {
  uv_close((uv_handle_t *) handle, appling_bootstrap__on_close);
}

int
appling_bootstrap(uv_loop_t *loop, js_platform_t *js, appling_bootstrap_t *req, const appling_key_t key, const char *dir, appling_bootstrap_cb cb, const appling_id_t link_key) {
  int err;

  req->loop = loop;
  req->js = js;
  req->cb = cb;
  req->status = 0;
  req->error = NULL;
  req->signal.data = (void *) req;

  err = uv_async_init(loop, &req->signal, appling_bootstrap__on_signal);
  if (err < 0) return err;

  memcpy(req->key, key, sizeof(appling_key_t));
  memcpy(req->link.id, link_key, sizeof(appling_id_t));

  if (dir && path_is_absolute(dir, path_behavior_system)) strcpy(req->dir, dir);
  else if (dir) {
    appling_path_t cwd;
    size_t path_len = sizeof(appling_path_t);

    err = uv_cwd(cwd, &path_len);
    if (err < 0) return err;

    path_len = sizeof(appling_path_t);

    path_join(
      (const char *[]) {cwd, dir, NULL},
      req->dir,
      &path_len,
      path_behavior_system
    );
  } else {
    appling_path_t homedir;
    size_t path_len = sizeof(appling_path_t);

    err = uv_os_homedir(homedir, &path_len);
    if (err < 0) return err;

    path_len = sizeof(appling_path_t);

    path_join(
      (const char *[]) {homedir, appling_platform_dir, NULL},
      req->dir,
      &path_len,
      path_behavior_system
    );
  }

  return uv_thread_create(&req->thread, appling_bootstrap__on_thread, (void *) req);
}
