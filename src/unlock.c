#include <fs.h>
#include <uv.h>

#include "../include/appling.h"

int
appling_unlock(appling_lock_t *req) {
  return fs_close(NULL, &req->close, req->file, NULL);
}
