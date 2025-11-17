#include <uv.h>

#include "../include/appling.h"

int
appling_unlock(appling_lock_t *req) {
  int err;

  uv_fs_t fs;
  err = uv_fs_close(NULL, &fs, req->file, NULL);
  if (err < 0) return err;

  return 0;
}
