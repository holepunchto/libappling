#include <hex.h>
#include <log.h>
#include <path.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

static void
on_process_exit (uv_process_t *handle, int64_t exit_status, int term_signal) {
  appling_process_t *process = (appling_process_t *) handle;

  if (process->on_exit) process->on_exit(process, exit_status, term_signal);
}

static inline void
get_app_root (const char *exe, char *result) {
#if defined(APPLING_OS_LINUX)
  const char *app = getenv("APPIMAGE");
  strcpy(result, app ? app : exe);
#elif defined(APPLING_OS_WIN32)
  strcpy(result, exe);
#elif defined(APPLING_OS_DARWIN)
  size_t path_len = PATH_MAX;
  path_join(
    (const char *[]){exe, "..", "..", "..", NULL},
    result,
    &path_len,
    path_behavior_system
  );
#endif
}

int
appling_launch (uv_loop_t *loop, appling_process_t *process, const appling_link_t *link, const appling_app_t *app, appling_exit_cb cb) {
  process->on_exit = cb;
  process->process.data = (void *) process;

  char *launch = NULL;

  if (link != NULL) {
    launch = malloc(8 /* punch:// */ + APPLING_KEY_LEN * 2 + 1 /* / */ + strlen(link->data) + 1 /* NULL */);
    launch[0] = '\0';

    strcat(launch, "punch://");

    size_t len = 65;

    hex_encode(link->key, APPLING_KEY_LEN, (utf8_t *) &launch[8], &len);

    if (strlen(link->data)) {
      strcat(launch, "/");
      strcat(launch, link->data);
    }
  }

  if (launch) log_debug("appling_launch() launching link %s", launch);

  char app_root[PATH_MAX];
  get_app_root(app->exe, app_root);

  log_debug("appling_launch() launching application shell %s", app_root);

  uv_process_options_t options = {
    .exit_cb = on_process_exit,
    .file = app->platform.exe,
    .args = (char *[]){
      (char *) app->platform.exe,
      "--appling",
      app_root,
      launch ? "--launch" : NULL,
      launch,
      NULL,
    },
    .flags = UV_PROCESS_WINDOWS_HIDE,
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

  int err = uv_spawn(loop, &process->process, &options);

  free(launch);

  return err;
}
