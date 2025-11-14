#include <path.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>
#include <uv.h>

#include "../include/appling.h"

int
appling_ready(const appling_platform_t *platform, const appling_link_t *link) {
  int err;

  appling_path_t path;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]) {platform->path, "lib", appling_platform_entry, NULL},
    path,
    &path_len,
    path_behavior_system
  );

  uv_lib_t library;
  err = uv_dlopen(path, &library);
  if (err < 0) return err;

  appling_ready_cb ready;
  err = uv_dlsym(&library, "appling_ready_v0", (void **) &ready);
  if (err < 0) {
    uv_dlclose(&library);

    return 1; // May not exist
  }

  appling_ready_info_t info = {
    .version = 0,
    .path = path,
    .platform = platform,
    .link = link,
  };

  err = ready(&info);

  uv_dlclose(&library);

  return err;
}
