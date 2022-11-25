#include <fs.h>
#include <limits.h>
#include <path.h>
#include <uv.h>

#include "../include/holepunch.h"

static void
on_close (fs_close_t *fs_req, int status) {
  holepunch_lock_t *req = (holepunch_lock_t *) fs_req->data;

  if (req->status < 0) status = req->status;

  if (status >= 0) {
    req->on_lock(req, 0);
  } else {
    req->on_lock(req, status);
  }
}

static void
on_lock (fs_lock_t *fs_req, int status) {
  holepunch_lock_t *req = (holepunch_lock_t *) fs_req->data;

  if (status >= 0) {
    req->on_lock(req, 0);
  } else {
    req->status = status; // Propagate

    fs_close(req->loop, &req->close, req->file, on_close);
  }
}

static void
on_open (fs_open_t *fs_req, int status, uv_file file) {
  holepunch_lock_t *req = (holepunch_lock_t *) fs_req->data;

  if (status >= 0) {
    req->file = file;

    fs_lock(req->loop, &req->lock, req->file, 0, 0, false, on_lock);
  } else {
    req->on_lock(req, status);
  }
}

int
holepunch_lock (uv_loop_t *loop, holepunch_lock_t *req, const char *dir, holepunch_lock_cb cb) {
  req->loop = loop;
  req->on_lock = cb;
  req->file = -1;
  req->open.data = (void *) req;
  req->lock.data = (void *) req;

  char path[PATH_MAX];

  if (dir) strcpy(path, dir);
  else {
    char homedir[PATH_MAX];
    size_t homedir_len = PATH_MAX;

    int err = uv_os_homedir(homedir, &homedir_len);
    if (err < 0) return err;

    size_t path_len = PATH_MAX;

    path_join(
      (const char *[]){homedir, holepunch_dir, NULL},
      path,
      &path_len,
      path_separator_system
    );
  }

  return fs_open(req->loop, &req->open, path, 0, O_RDONLY, on_open);
}
