#include <fs.h>
#include <path.h>
#include <string.h>
#include <uv.h>

#include "../include/appling.h"

static void
appling_lock__on_close(fs_close_t *fs_req, int status) {
  appling_lock_t *req = (appling_lock_t *) fs_req->data;

  if (req->status < 0) status = req->status;

  if (status >= 0) {
    if (req->on_lock) req->on_lock(req, 0);
  } else {
    if (req->on_lock) req->on_lock(req, status);
  }
}

static void
appling_lock__on_lock(fs_lock_t *fs_req, int status) {
  appling_lock_t *req = (appling_lock_t *) fs_req->data;

  if (status >= 0) {
    if (req->on_lock) req->on_lock(req, 0);
  } else {
    req->status = status; // Propagate

    fs_close(req->loop, &req->close, req->file, appling_lock__on_close);
  }
}

static void
appling_lock__on_open(fs_open_t *fs_req, int status, uv_file file) {
  appling_lock_t *req = (appling_lock_t *) fs_req->data;

  if (status >= 0) {
    req->file = file;

    fs_lock(req->loop, &req->lock, req->file, 0, 0, false, appling_lock__on_lock);
  } else {
    if (req->on_lock) req->on_lock(req, status);
  }
}

static void
appling_lock__on_mkdir(fs_mkdir_t *fs_req, int status) {
  appling_lock_t *req = (appling_lock_t *) fs_req->data;

  appling_path_t path;
  size_t path_len = sizeof(appling_path_t);

  path_join(
    (const char *[]) {req->dir, "lock", NULL},
    path,
    &path_len,
    path_behavior_system
  );

  if (status >= 0) {
    fs_open(req->loop, &req->open, path, UV_FS_O_RDWR | UV_FS_O_CREAT, 0666, appling_lock__on_open);
  } else {
    if (req->on_lock) req->on_lock(req, status);
  }
}

int
appling_lock(uv_loop_t *loop, appling_lock_t *req, const char *dir, appling_lock_cb cb) {
  int err;

  req->loop = loop;
  req->on_lock = cb;
  req->file = -1;
  req->mkdir.data = (void *) req;
  req->open.data = (void *) req;
  req->lock.data = (void *) req;
  req->close.data = (void *) req;

  if (dir && path_is_absolute(dir, path_behavior_system)) strcpy(req->dir, dir);
  else if (dir) {
    appling_path_t cwd;
    size_t path_len = sizeof(appling_path_t);

    err = uv_cwd(cwd, &path_len);
    if (err < 0) return err;

    path_len = sizeof(appling_path_t);

    path_join(
      (const char *[]) {cwd, dir, NULL},
      req->dir,
      &path_len,
      path_behavior_system
    );
  } else {
    appling_path_t homedir;
    size_t path_len = sizeof(appling_path_t);

    err = uv_os_homedir(homedir, &path_len);
    if (err < 0) return err;

    path_len = sizeof(appling_path_t);

    path_join(
      (const char *[]) {homedir, appling_platform_dir, NULL},
      req->dir,
      &path_len,
      path_behavior_system
    );
  }

  return fs_mkdir(req->loop, &req->mkdir, req->dir, 0777, true, appling_lock__on_mkdir);
}
