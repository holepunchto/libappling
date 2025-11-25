#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <uv.h>

#include "../include/appling.h"

uv_loop_t *loop;

appling_platform_t platform;

appling_resolve_t req;

static void
on_progress(const appling_progress_info_t *info) {
  printf(
    "progress\n  peers=%lld\n  upload_speed=%f\n  uploaded_bytes=%lld\n  uploaded_blocks=%lld\n  download_speed=%f\n  download_progress=%f\n  downloaded_bytes=%lld\n  downloaded_blocks=%lld\n",
    info->peers,
    info->upload_speed,
    info->uploaded_bytes,
    info->uploaded_blocks,
    info->download_speed,
    info->download_progress,
    info->downloaded_bytes,
    info->downloaded_blocks
  );
}

static void
on_resolve(appling_resolve_t *req, int status) {
  int err;

  assert(status == 0);

  appling_link_t link = {
    .id = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
  };

  err = appling_preflight(&platform, &link, on_progress);
  assert(err == 0);
}

int
main() {
  int err;

  loop = uv_default_loop();

  err = appling_resolve(loop, &req, "test/fixtures/platform", &platform, on_resolve);
  assert(err == 0);

  uv_run(loop, UV_RUN_DEFAULT);

  return 0;
}
