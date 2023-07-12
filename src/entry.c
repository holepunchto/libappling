#include <assert.h>
#include <hex.h>
#include <log.h>
#include <path.h>
#include <stdint.h>
#include <string.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

static void
on_process_exit (uv_process_t *handle, int64_t exit_status, int term_signal) {
  *((int64_t *) handle->data) = exit_status;

  uv_close((uv_handle_t *) handle, NULL);
}

int
appling_launch_v0 (const appling_launch_info_t *info) {
  int err;

  const appling_platform_t *platform = info->platform;

  appling_path_t file;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]) {
      platform->path,
        "bin",
#if defined(APPLING_OS_LINUX)
        "holepunch-runtime/holepunch-runtime",
#elif defined(APPLING_OS_WIN32)
        "holepunch-runtime\\Holepunch Runtime.exe",
#elif defined(APPLING_OS_DARWIN)
        "Holepunch Runtime.app/Contents/MacOS/Holepunch Runtime",
#endif
        NULL,
    },
    file,
    &path_len,
    path_behavior_system
  );

  const appling_app_t *app = info->app;

  appling_path_t appling;

#if defined(APPLING_OS_LINUX)
  appling_path_t appimage;
  path_len = sizeof(appling_path_t);

  err = uv_os_getenv("APPIMAGE", appimage, &path_len);

  strcpy(appling, err == 0 ? appimage : app->path);
#else
  strcpy(appling, app->path);
#endif

  log_debug("appling_launch() launching application shell %s", appling);

  const appling_link_t *link = info->link;

  char launch[7 /* pear:// */ + APPLING_KEY_LEN * 2 + 1 /* / */ + APPLING_LINK_DATA_MAX + 1 /* NULL */] = {'\0'};

  strcat(launch, "pear://");

  size_t len = 65;

  hex_encode(link->key, APPLING_KEY_LEN, (utf8_t *) &launch[7], &len);

  if (strlen(link->data)) {
    strcat(launch, "/");
    strcat(launch, link->data);
  }

  log_debug("appling_launch() launching link %s", launch);

  char *args[] = {file, "--appling", appling, "--launch", launch, NULL};

  uv_loop_t loop;
  err = uv_loop_init(&loop);
  if (err < 0) goto err;

  int64_t exit_status = 0;

  uv_process_t process;

  process.data = (void *) &exit_status;

  uv_process_options_t options = {
    .exit_cb = on_process_exit,
    .file = file,
    .args = args,
    .flags = UV_PROCESS_WINDOWS_HIDE,
    .stdio_count = 3,
    .stdio = (uv_stdio_container_t[]){
      {.flags = UV_INHERIT_FD, .data.fd = 0},
      {.flags = UV_INHERIT_FD, .data.fd = 1},
      {.flags = UV_INHERIT_FD, .data.fd = 2},
    },
  };

  err = uv_spawn(&loop, &process, &options);
  if (err < 0) goto err;

  uv_run(&loop, UV_RUN_DEFAULT);

  err = uv_loop_close(&loop);
  assert(err == 0);

  return exit_status;

err:
  err = uv_loop_close(&loop);
  assert(err == 0);

  return err;
}
