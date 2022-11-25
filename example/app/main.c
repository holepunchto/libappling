#include <assert.h>
#include <holepunch.h>
#include <stdint.h>
#include <uv.h>

#ifndef EXAMPLE_HOLEPUNCH_DIR
#define EXAMPLE_HOLEPUNCH_DIR "example/holepunch"
#endif

uv_loop_t *loop;

holepunch_lock_t lock;
holepunch_resolve_t resolve;
holepunch_bootstrap_t bootstrap;
holepunch_process_t process;

static void
on_exit (holepunch_process_t *process, int64_t exit_status, int term_signal) {
  assert(exit_status == 0);
}

static void
on_unlock (holepunch_lock_t *req, int status) {
  assert(status == 0);
}

static void
on_bootstrap (holepunch_bootstrap_t *req, int status, const holepunch_app_t *app) {
  assert(status == 0);

  holepunch_unlock(req->loop, &lock, on_unlock);

  status = holepunch_launch(req->loop, &process, NULL, app, on_exit);

  assert(status == 0);
}

static void
on_resolve (holepunch_resolve_t *req, int status, const holepunch_platform_t *platform) {
  holepunch_bootstrap(req->loop, &bootstrap, NULL, EXAMPLE_HOLEPUNCH_DIR, platform, on_bootstrap);
}

static void
on_lock (holepunch_lock_t *req, int status) {
  assert(status == 0);

  holepunch_resolve(loop, &resolve, EXAMPLE_HOLEPUNCH_DIR, on_resolve);
}

int
main (int argc, char *argv[]) {
  uv_setup_args(argc, argv);

  loop = uv_default_loop();

  holepunch_lock(loop, &lock, EXAMPLE_HOLEPUNCH_DIR, on_lock);

  return uv_run(loop, UV_RUN_DEFAULT);
}
