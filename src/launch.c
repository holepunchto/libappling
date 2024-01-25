#include <js.h>
#include <log.h>
#include <path.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

int
appling_launch (uv_loop_t *loop, js_platform_t *js, const appling_platform_t *platform, const appling_app_t *app, const appling_link_t *link) {
  int err;

  appling_path_t path;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]){platform->path, "lib", appling_platform_entry, NULL},
    path,
    &path_len,
    path_behavior_system
  );

  uv_lib_t library;
  err = uv_dlopen(path, &library);
  if (err < 0) return err;

  appling_launch_cb launch;
  err = uv_dlsym(&library, "appling_launch_v" APPLING_STRING(APPLING_VERSION_MAJOR), (void **) &launch);
  if (err < 0) return err;

  appling_launch_info_t info = {
    .version = 1,
    .path = path,
    .platform = platform,
    .app = app,
    .link = link,
    .js = js,
  };

  return launch(&info);
}
