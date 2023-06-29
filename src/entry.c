#include <assert.h>
#include <hex.h>
#include <log.h>
#include <path.h>
#include <stdint.h>
#include <string.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

typedef struct appling_app_with_link_s appling_app_with_link_t;

struct appling_app_with_link_s {
  appling_app_t app;
  appling_link_t link;
};

/**
 * @todo This should be read from a configuration file somewhere.
 */
static const appling_app_with_link_t apps[] = {
  {
    .app = {
#if defined(APPLING_OS_DARWIN)
      .path = "/Applications/Keet.app/Contents/MacOS/Keet",
#endif
    },
    .link = {
      .key = {0x4b, 0x32, 0x78, 0xfc, 0x44, 0xe9, 0x71, 0x6c, 0x03, 0x42, 0x71, 0x5f, 0x42, 0xe3, 0x14, 0x05, 0x0a, 0x3c, 0x82, 0x5a, 0x51, 0x05, 0x6a, 0xc5, 0x3e, 0xe8, 0x17, 0x09, 0x86, 0xa8, 0xbb, 0x86},
    },
  },
};

static void
on_process_exit (uv_process_t *handle, int64_t exit_status, int term_signal) {
  *((int64_t *) handle->data) = exit_status;

  uv_close((uv_handle_t *) handle, NULL);
}

int
appling_launch_v0 (const appling_launch_info_t *info) {
  const appling_platform_t *platform = info->platform;
  const appling_app_t *app = info->app;
  const appling_link_t *link = info->link;

  if (app == NULL || link == NULL) {
    for (size_t i = 0, n = sizeof(apps) / sizeof(appling_app_with_link_t); i < n; i++) {
      if (app == NULL && memcmp(apps[i].link.key, link->key, sizeof(appling_key_t)) == 0) {
        app = &apps[i].app;
        break;
      }

      if (link == NULL && strcmp(apps[i].app.path, app->path) == 0) {
        link = &apps[i].link;
        break;
      }
    }

    if (app == NULL || link == NULL) return -1;
  }

  appling_path_t file;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]) {
      platform->path,
        "bin",
#if defined(APPLING_OS_LINUX)
        "holepunch-runtime/holepunch",
#elif defined(APPLING_OS_WIN32)
        "holepunch-runtime\\Holepunch Runtime.exe",
#elif defined(APPLING_OS_DARWIN)
        "Holepunch.app/Contents/MacOS/Holepunch",
#endif
        NULL,
    },
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
