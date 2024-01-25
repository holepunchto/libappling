#include <assert.h>
#include <js.h>
#include <string.h>
#include <uv.h>

#include "../include/appling.h"

typedef struct appling_main_s appling_main_t;

struct appling_main_s {
  js_platform_t *js;

  const char *dir;
  appling_platform_t *platform;
  appling_app_t *app;

  appling_link_t link;

  appling_lock_t lock;
  appling_resolve_t resolve;
  appling_bootstrap_t bootstrap;
};

static void
on_unlock (appling_lock_t *req, int status) {
  assert(status == 0);
}

static void
on_resolve (appling_resolve_t *req, int status);

static void
on_bootstrap (appling_bootstrap_t *req, int status) {
  appling_main_t *main = (appling_main_t *) req->data;

  assert(status == 0);

  status = appling_resolve(req->loop, &main->resolve, main->dir, main->platform, on_resolve);
  assert(status == 0);
}

static void
on_resolve (appling_resolve_t *req, int status) {
  appling_main_t *main = (appling_main_t *) req->data;

  if (status == 0) {
    status = appling_unlock(req->loop, &main->lock, on_unlock);
    assert(status == 0);

    status = appling_launch(req->loop, main->js, main->platform, main->app, &main->link);
    assert(status == 0);
  } else {
    status = appling_bootstrap(req->loop, main->js, &main->bootstrap, main->platform->dkey, main->app->path, main->dir, on_bootstrap);
    assert(status == 0);
  }
}

static void
on_lock (appling_lock_t *req, int status) {
  appling_main_t *main = (appling_main_t *) req->data;

  assert(status == 0);

  status = appling_resolve(req->loop, &main->resolve, main->dir, main->platform, on_resolve);
  assert(status == 0);
}

int
appling_main (int argc, char *argv[], const char *dir, appling_platform_t *platform, appling_app_t *app) {
  int err;

  appling_main_t main = {
    .dir = dir,
    .platform = platform,
    .app = app,
  };

  main.lock.data = (void *) &main;
  main.resolve.data = (void *) &main;
  main.bootstrap.data = (void *) &main;

  argv = uv_setup_args(argc, argv);

  size_t path_len = sizeof(appling_path_t);

  err = uv_exepath(app->path, &path_len);
  assert(err == 0);

  uv_loop_t loop;
  err = uv_loop_init(&loop);
  assert(err == 0);

  if (argc > 1) {
    err = appling_parse(argv[1], &main.link);
    assert(err == 0);
  } else {
    memcpy(&main.link.key, app->key, sizeof(appling_key_t));
  }

  err = js_create_platform(&loop, NULL, &main.js);
  assert(err == 0);

  err = appling_lock(&loop, &main.lock, main.dir, on_lock);
  assert(err == 0);

  err = uv_run(&loop, UV_RUN_DEFAULT);
  assert(err == 0);

  err = js_destroy_platform(main.js);
  assert(err == 0);

  err = uv_run(&loop, UV_RUN_DEFAULT);
  assert(err == 0);

  err = uv_loop_close(&loop);
  assert(err == 0);

  return 0;
}
