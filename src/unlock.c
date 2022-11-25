#include <fs.h>
#include <uv.h>

#include "../include/appling.h"

static void
on_close (fs_close_t *fs_req, int status) {
  appling_lock_t *req = (appling_lock_t *) fs_req->data;

  if (status >= 0) {
    req->on_unlock(req, 0);
  } else {
    req->on_unlock(req, status);
  }
}

int
appling_unlock (uv_loop_t *loop, appling_lock_t *req, appling_unlock_cb cb) {
  req->loop = loop;
  req->on_unlock = cb;
  req->close.data = (void *) req;

  return fs_close(req->loop, &req->close, req->file, on_close);
}
