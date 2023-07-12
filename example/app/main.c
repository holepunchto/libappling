#include <appling.h>
#include <assert.h>
#include <log.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <uv.h>

#ifndef PLATFORM_DIR
#define PLATFORM_DIR "example/pear"
#endif

uv_loop_t *loop;

appling_lock_t lock;
appling_resolve_t resolve;
appling_bootstrap_t bootstrap;

appling_platform_t platform = {
  .key = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa},
};

appling_app_t app = {
  .key = {0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb},
};

appling_link_t link;

static void
on_resolve (appling_resolve_t *req, int status);

static void
on_unlock (appling_lock_t *req, int status) {
  assert(status == 0);
}

static void
on_bootstrap (appling_bootstrap_t *req, int status) {
  assert(status == 0);

  status = appling_resolve(loop, &resolve, PLATFORM_DIR, &platform, on_resolve);
  assert(status == 0);
}

static void
on_resolve (appling_resolve_t *req, int status) {
  if (status == 0) {
    status = appling_unlock(req->loop, &lock, on_unlock);
    assert(status == 0);

    status = appling_launch(req->loop, &platform, &app, &link);
    assert(status == 0);
  } else {
    status = appling_bootstrap(req->loop, &bootstrap, platform.key, app.path, PLATFORM_DIR, on_bootstrap);
    assert(status == 0);
  }
}

static void
on_lock (appling_lock_t *req, int status) {
  assert(status == 0);

  status = appling_resolve(loop, &resolve, PLATFORM_DIR, &platform, on_resolve);
  assert(status == 0);
}

int
main (int argc, char *argv[]) {
  int err;

  err = log_open("Appling", 0);
  assert(err == 0);

  argv = uv_setup_args(argc, argv);

  size_t path_len = sizeof(appling_path_t);

  err = uv_exepath(app.path, &path_len);
  assert(err == 0);

  loop = uv_default_loop();

  if (argc > 1) {
    err = appling_parse(argv[1], &link);
    assert(err == 0);
  } else {
    memcpy(link.key, app.key, sizeof(appling_key_t));
  }

  err = appling_lock(loop, &lock, PLATFORM_DIR, on_lock);
  assert(err == 0);

  return uv_run(loop, UV_RUN_DEFAULT);
}
