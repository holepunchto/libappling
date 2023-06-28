#include <hex.h>
#include <log.h>
#include <path.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

#if defined(APPLING_OS_LINUX)
#define APPLING_PLATFORM_EXE "holepunch-runtime/holepunch"
#elif defined(APPLING_OS_WIN32)
#define APPLING_PLATFORM_EXE "holepunch-runtime\\Holepunch Runtime.exe"
#elif defined(APPLING_OS_DARWIN)
#define APPLING_PLATFORM_EXE "Holepunch.app/Contents/MacOS/Holepunch"
#endif

static void
on_process_exit (uv_process_t *handle, int64_t exit_status, int term_signal) {
  *((int64_t *) handle->data) = exit_status;
}

int
appling_launch_v0 (const appling_launch_info_t *info) {
  const appling_link_t *link = info->link;

  char *launch = NULL;

  if (link) {
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

  const appling_app_t *app = info->app;

  appling_path_t file;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){app->platform.path, "bin", APPLING_PLATFORM_EXE, NULL},
    file,
    &path_len,
    path_behavior_system
  );

  appling_path_t appling;

#if defined(APPLING_OS_LINUX)
  const char *appimage = getenv("APPIMAGE");

  strcpy(appling, appimage ? appimage : app->path);
#elif defined(APPLING_OS_WIN32)
  strcpy(appling, app->path);
#elif defined(APPLING_OS_DARWIN)
  path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){app->path, "..", "..", "..", NULL},
    appling,
    &path_len,
    path_behavior_system
  );
#endif

  log_debug("appling_launch() launching application shell %s", appling);

  int err;

  uv_loop_t loop;
  err = uv_loop_init(&loop);
  if (err < 0) goto err;

  int64_t exit_status = 0;

  uv_process_t process;

  process.data = (void *) &exit_status;

  uv_process_options_t options = {
    .exit_cb = on_process_exit,
    .file = file,
    .args = (char *[]){
      file,
      "--appling",
      appling,
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

  err = uv_spawn(&loop, &process, &options);
  if (err < 0) goto err;

  if (launch) free(launch);

  uv_run(&loop, UV_RUN_DEFAULT);

  return exit_status;

err:
  if (launch) free(launch);

  return err;
}
