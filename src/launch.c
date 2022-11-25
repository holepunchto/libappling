#include <path.h>
#include <stdint.h>
#include <stdlib.h>
#include <uv.h>

#include "../include/holepunch.h"

static void
on_exit (uv_process_t *handle, int64_t exit_status, int term_signal) {
  holepunch_process_t *process = (holepunch_process_t *) handle;

  process->on_exit(process, exit_status, term_signal);
}

static inline void
get_app_root (const char *exe, char *result) {
#if defined(HOLEPUNCH_OS_LINUX)
  const char *app = getenv("APPIMAGE");
  strcpy(result, app ? app : exe);
#elif defined(HOLEPUNCH_OS_WIN32)
  strcpy(result, exe);
#elif defined(HOLEPUNCH_OS_DARWIN)
  size_t path_len = PATH_MAX;
  path_join(
    (const char *[]){exe, "..", "..", "..", NULL},
    result,
    &path_len,
    path_separator_system
  );
#endif
}

int
holepunch_launch (uv_loop_t *loop, holepunch_process_t *process, const holepunch_link_t *link, const holepunch_app_t *app, holepunch_exit_cb cb) {
  process->on_exit = cb;
  process->process.data = (void *) process;

  char *key;

  if (link == NULL) key = (char *) app->key;
  else {
    key = malloc(8 /* punch:// */ + strlen(link->key) + 1 /* / */ + strlen(link->data) + 1 /* NULL */);
    key[0] = '\0';

    strcat(key, "punch://");
    strcat(key, link->key);

    if (strlen(link->data)) {
      strcat(key, "/");
      strcat(key, link->data);
    }
  }

  char app_root[PATH_MAX];
  get_app_root(app->exe, app_root);

  uv_process_options_t options = {
    .exit_cb = on_exit,
    .file = app->platform.exe,
    .args = (char *[]){
      (char *) app->platform.exe,
      "--appling",
      app_root,
      "--no-multiapp",
      "--sequester",
      "--launch",
      key,
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

  int err = uv_spawn(loop, &process->process, &options);

  if (link) free(key);

  return err;
}
