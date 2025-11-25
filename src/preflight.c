#include <assert.h>
#include <path.h>
#include <stdint.h>
#include <stdlib.h>
#include <uv.h>

#include "../include/appling.h"

int
appling_preflight(const appling_platform_t *platform, const appling_link_t *link, appling_progress_cb cb) {
  int err;

  appling_path_t path;
  size_t path_len = sizeof(appling_path_t);

  err = path_join(
    (const char *[]) {platform->path, "lib", appling_platform_entry, NULL},
    path,
    &path_len,
    path_behavior_system
  );
  assert(err == 0);

  uv_lib_t library;
  err = uv_dlopen(path, &library);
  if (err < 0) return err;

  appling_preflight_cb preflight;
  err = uv_dlsym(&library, "appling_preflight_v0", (void **) &preflight);
  if (err < 0) {
    uv_dlclose(&library);

    return 0; // May not exist
  }

  appling_preflight_info_t info = {
    .version = 0,
    .path = path,
    .platform = platform,
    .link = link,
    .progress = cb,
  };

  err = preflight(&info);

  uv_dlclose(&library);

  return err;
}
