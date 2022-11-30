#include <fs.h>
#include <path.h>
#include <uv.h>

#include "../include/appling.h"

static void
on_close (fs_close_t *fs_req, int status) {
  appling_lock_t *req = (appling_lock_t *) fs_req->data;

  if (req->status < 0) status = req->status;

  if (status >= 0) {
    req->on_lock(req, 0);
  } else {
    req->on_lock(req, status);
  }
}

static void
on_lock (fs_lock_t *fs_req, int status) {
  appling_lock_t *req = (appling_lock_t *) fs_req->data;

  if (status >= 0) {
    req->on_lock(req, 0);
  } else {
    req->status = status; // Propagate

    fs_close(req->loop, &req->close, req->file, on_close);
  }
}

static void
on_open (fs_open_t *fs_req, int status, uv_file file) {
  appling_lock_t *req = (appling_lock_t *) fs_req->data;

  if (status >= 0) {
    req->file = file;

    fs_lock(req->loop, &req->lock, req->file, 0, 0, false, on_lock);
  } else {
    req->on_lock(req, status);
  }
}

static void
on_mkdir (fs_mkdir_t *fs_req, int status) {
  appling_lock_t *req = (appling_lock_t *) fs_req->data;

  char path[PATH_MAX];
  size_t path_len = PATH_MAX;

  path_join(
    (const char *[]){req->dir, "lock", NULL},
    path,
    &path_len,
    path_behavior_system
  );

  if (status >= 0) {
    fs_open(req->loop, &req->open, path, UV_FS_O_RDWR | UV_FS_O_CREAT, 0666, on_open);
  } else {
    req->on_lock(req, status);
  }
}

int
appling_lock (uv_loop_t *loop, appling_lock_t *req, const char *dir, appling_lock_cb cb) {
  req->loop = loop;
  req->on_lock = cb;
  req->file = -1;
  req->mkdir.data = (void *) req;
  req->open.data = (void *) req;
  req->lock.data = (void *) req;
  req->close.data = (void *) req;

  if (dir) strcpy(req->dir, dir);
  else {
    char homedir[PATH_MAX];
    size_t homedir_len = PATH_MAX;

    int err = uv_os_homedir(homedir, &homedir_len);
    if (err < 0) return err;

    size_t path_len = PATH_MAX;

    path_join(
      (const char *[]){homedir, appling_platform_dir, NULL},
      req->dir,
      &path_len,
      path_behavior_system
    );
  }

  return fs_mkdir(req->loop, &req->mkdir, req->dir, 0777, true, on_mkdir);
}
