#include <stdint.h>
#include <uv.h>

#include "../include/holepunch.h"

static void
on_exit (uv_process_t *handle, int64_t exit_status, int term_signal) {
  holepunch_process_t *process = (holepunch_process_t *) handle;

  if (process->on_exit) process->on_exit(process, exit_status, term_signal);
}

int
holepunch_launch (uv_loop_t *loop, holepunch_process_t *process, const holepunch_link_t *link, const holepunch_app_t *app, holepunch_exit_cb cb) {
  process->on_exit = cb;
  process->process.data = (void *) process;

  uv_process_options_t options = {
    .exit_cb = on_exit,
    .file = app->platform.exe,
    .args = (char *[]){
      (char *) app->platform.exe,
      "--no-multiapp",
      "--sequester",
      "--launch",
      (char *) app->key,
      NULL,
    },
    .stdio_count = 3,
    .stdio = (uv_stdio_container_t[]){
      {
        .flags = UV_INHERIT_FD,
        .data.fd = 0,
      },
      {
        .flags = UV_INHERIT_FD,
        .data.fd = 1,
      },
      {
        .flags = UV_INHERIT_FD,
        .data.fd = 2,
      },
    },
  };

  return uv_spawn(loop, &process->process, &options);
}
