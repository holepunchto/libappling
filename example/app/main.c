#include <appling.h>
#include <assert.h>
#include <stdint.h>
#include <uv.h>

#ifndef PLATFORM_DIR
#define PLATFORM_DIR "example/holepunch"
#endif

uv_loop_t *loop;

appling_lock_t lock;
appling_resolve_t resolve;
appling_bootstrap_t bootstrap;
appling_process_t process;

static void
on_process_exit (appling_process_t *process, int64_t exit_status, int term_signal) {
  assert(exit_status == 0);
}

static void
on_unlock (appling_lock_t *req, int status) {
  assert(status == 0);
}

static void
on_bootstrap (appling_bootstrap_t *req, int status, const appling_app_t *app) {
  assert(status == 0);

  appling_unlock(req->loop, &lock, on_unlock);

  status = appling_launch(req->loop, &process, NULL, app, on_process_exit);

  assert(status == 0);
}

static void
on_resolve (appling_resolve_t *req, int status, const appling_platform_t *platform) {
  appling_bootstrap(req->loop, &bootstrap, NULL, PLATFORM_DIR, platform, on_bootstrap);
}

static void
on_lock (appling_lock_t *req, int status) {
  assert(status == 0);

  appling_resolve(loop, &resolve, PLATFORM_DIR, on_resolve);
}

int
main (int argc, char *argv[]) {
  uv_setup_args(argc, argv);

  loop = uv_default_loop();

  appling_lock(loop, &lock, PLATFORM_DIR, on_lock);

  return uv_run(loop, UV_RUN_DEFAULT);
}
